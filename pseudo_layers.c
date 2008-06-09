/*
	:flock - open with flock()
	:creat - open with O_CREAT
	:excl  - open with O_EXCL

*/

#include "perlioutil.h"
#include "perlioflock.h"

static IV
PerlIOFlock_pushed(pTHX_ PerlIO* fp, const char* mode, SV* arg,
		PerlIO_funcs* tab){

	int lock_mode;
	int fd;
	int ret;

	PERL_UNUSED_ARG(mode);
	PERL_UNUSED_ARG(tab);

	if(!PerlIOValid(fp)){
		SETERRNO(EBADF, SS_IVCHAN);
		return -1;
	}

	lock_mode = IOLflag(fp, PERLIO_F_CANWRITE) ? LOCK_EX : LOCK_SH;

	if(arg && SvOK(arg)){
		const char* blocking = SvPV_nolen(arg);

		if(strEQ(blocking, "blocking")){
			/* noop */
		}
		else if(strEQ(blocking, "non-blocking")
			|| strEQ(blocking, "LOCK_NB")){
			lock_mode |= LOCK_NB;
		}
		else{
			Perl_croak(aTHX_ "Unrecognized :flock handler '%s' "
				"(it must be 'blocking' or 'non-blocking')",
					blocking);
		}
	}

	fd  = PerlIO_fileno(fp);
	if(fd == -1){ /* :scalar, :dir, etc. */
		return 0; /* success */
	}

	PerlIO_flush(fp);
	ret = PerlLIO_flock(fd, lock_mode);

	PerlIO_debug(STRINGIFY(FLOCK) "(%d, %s) -> %d\n", fd,
		(  lock_mode == (LOCK_SH)         ? "LOCK_SH"
		 : lock_mode == (LOCK_SH|LOCK_NB) ? "LOCK_SH|LOCK_NB"
		 : lock_mode == (LOCK_EX)         ? "LOCK_EX"
		 : lock_mode == (LOCK_EX|LOCK_NB) ? "LOCK_EX|LOCK_NB"
		 : "(UNKNOWN)" ),
		ret);

	return ret;
}

static IV
useless_pushed(pTHX_ PerlIO* fp, const char* mode, SV* arg,
		PerlIO_funcs* tab){
	PERL_UNUSED_ARG(fp);
	PERL_UNUSED_ARG(mode);
	PERL_UNUSED_ARG(arg);

	if(ckWARN(WARN_LAYER)){
		Perl_warner(aTHX_ packWARN(WARN_LAYER),
			"Too late for %s layer", tab->name);
	}

	return 0;
}

static PerlIO*
PerlIOUtil_open_with_flags(pTHX_ PerlIO_funcs* self, PerlIO_list_t* layers, IV n,
		const char* mode, int fd, int imode, int perm,
		PerlIO* f, int narg, SV** args, int flags){
	PerlIO_funcs* tab = NULL;
	char numeric_mode[5]; /* [I#]? [wra]\+? [tb] \0 */
	IV i;

	PERL_UNUSED_ARG(self);

	if(mode[0] != IoTYPE_NUMERIC){
		assert( sizeof(numeric_mode) > strlen(mode) );
		numeric_mode[0] = IoTYPE_NUMERIC;
		Copy(mode, &numeric_mode[1], strlen(mode), char*);
		mode = &numeric_mode[0];
	}

	if(imode){
		imode |= flags;
	}
	else{
		imode = PerlIOUnix_oflags(mode) | flags;
		perm = 0666;
	}

	i = n;
	while(--i >= 0){
		tab = LayerFetch(layers, i);
		if(tab && tab->Open){
			break;
		}
	}

	if(tab && tab->Open){
		f = tab->Open(aTHX_ tab, layers, i,  mode,
				fd, imode, perm, f, narg, args);

		/* apply above layers
		   e.g. [ :perlio :creat :utf8 :excl ]
		                         ~~~~~        
		*/

		if(f && ++i < n){
			/*print_layer_list(aTHX_ layers, i, n);*/
			if(PerlIO_apply_layera(aTHX_ f, mode, layers, i, n) != 0){
				PerlIO_close(f);
				f = NULL;
			}
		}

	}

	return f;
}

static PerlIO*
PerlIOCreat_open(pTHX_ PerlIO_funcs* self, PerlIO_list_t* layers, IV n,
		  const char* mode, int fd, int imode, int perm,
		  PerlIO* f, int narg, SV** args){

	return PerlIOUtil_open_with_flags(aTHX_ self, layers, n, mode, fd, imode,
			perm, f, narg, args, O_CREAT);
}

static PerlIO*
PerlIOExcl_open(pTHX_ PerlIO_funcs* self, PerlIO_list_t* layers, IV n,
		  const char* mode, int fd, int imode, int perm,
		  PerlIO* f, int narg, SV** args){

	return PerlIOUtil_open_with_flags(aTHX_ self, layers, n, mode, fd, imode,
			perm, f, narg, args, O_EXCL);
}


/* :flock */
PERLIO_FUNCS_DECL(PerlIO_flock) = {
	sizeof(PerlIO_funcs),
	"flock",
	0, /* size */
	PERLIO_K_DUMMY, /* kind */
	PerlIOFlock_pushed,
	NULL, /* popped */
	NULL, /* open */
	NULL, /* binmode */
	NULL, /* arg */
	NULL, /* fileno */
	NULL, /* dup */
	NULL, /* read */
	NULL, /* unread */
	NULL, /* write */
	NULL, /* seek */
	NULL, /* tell */
	NULL, /* close */
	NULL, /* flush */
	NULL, /* fill */
	NULL, /* eof */
	NULL, /* error */
	NULL, /* clearerr */
	NULL, /* setlinebuf */
	NULL, /* get_base */
	NULL, /* bufsiz */
	NULL, /* get_ptr */
	NULL, /* get_cnt */
	NULL  /* set_ptrcnt */
};

/* :creat */
PERLIO_FUNCS_DECL(PerlIO_creat) = {
	sizeof(PerlIO_funcs),
	"creat",
	0, /* size */
	PERLIO_K_DUMMY, /* kind */
	useless_pushed,
	NULL, /* popped */
	PerlIOCreat_open,
	NULL, /* binmode */
	NULL, /* arg */
	NULL, /* fileno */
	NULL, /* dup */
	NULL, /* read */
	NULL, /* unread */
	NULL, /* write */
	NULL, /* seek */
	NULL, /* tell */
	NULL, /* close */
	NULL, /* flush */
	NULL, /* fill */
	NULL, /* eof */
	NULL, /* error */
	NULL, /* clearerr */
	NULL, /* setlinebuf */
	NULL, /* get_base */
	NULL, /* bufsiz */
	NULL, /* get_ptr */
	NULL, /* get_cnt */
	NULL  /* set_ptrcnt */
};

/* :excl */
PERLIO_FUNCS_DECL(PerlIO_excl) = {
	sizeof(PerlIO_funcs),
	"excl",
	0, /* size */
	PERLIO_K_DUMMY, /* kind */
	useless_pushed,
	NULL, /* popped */
	PerlIOExcl_open,
	NULL, /* binmode */
	NULL, /* arg */
	NULL, /* fileno */
	NULL, /* dup */
	NULL, /* read */
	NULL, /* unread */
	NULL, /* write */
	NULL, /* seek */
	NULL, /* tell */
	NULL, /* close */
	NULL, /* flush */
	NULL, /* fill */
	NULL, /* eof */
	NULL, /* error */
	NULL, /* clearerr */
	NULL, /* setlinebuf */
	NULL, /* get_base */
	NULL, /* bufsiz */
	NULL, /* get_ptr */
	NULL, /* get_cnt */
	NULL  /* set_ptrcnt */
};

