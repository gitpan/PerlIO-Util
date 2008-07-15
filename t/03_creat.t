#!perl
use strict;
use warnings;
use Test::More tests => 21;

use FindBin qw($Bin);
use File::Spec;
BEGIN{
	eval 'use Fcntl;1' or *O_RDWR = sub(){ 2 };
}

use Fatal qw(unlink);

#use subs 'open';
#sub open(*;$@){
#	my($fh, $layers, @arg) = @_;
#	no strict 'refs';
#	my $st = CORE::open(*$fh, $layers, @arg);
#	if(!$st){
#		diag "open failed: $!";
#	}
#	return $st;
#}

my $file = File::Spec->join($Bin, 'util', '.creat');

ok !-e $file, "before open: the file doesn't exist";

ok open(*IN, "<:creat", $file), "open with :creat";

ok -e $file, "after open: the file does exist";


close *IN;
unlink $file;
ok open(*IN, "<:utf8 :creat", $file), "open with :utf8 :creat -> failure";
ok scalar(grep { $_ eq 'utf8' } *IN->get_layers()), 'utf8 on';
ok -e $file, "... not exist";

ok open(*IN, "<:creat :utf8", $file), "open with :creat :utf8";
ok -e $file, "... exist";

close *IN;
unlink $file;
ok open(*IN, "<:raw :creat", $file), "open with :raw :creat";
ok -e $file, "... exist";


close *IN;
unlink $file;
ok open(*IN, "<:unix :creat", $file), "open with :unix :creat";
ok -e $file, "... exist";


close *IN;
unlink $file;
ok open(*IN, "<:crlf :creat", $file), "open with :crlf :creat";
ok -e $file, "... exist";

close *IN;
unlink $file;
ok open(*IN, "<:creat :crlf", $file), "open with :creat :crlf";
ok -e $file, "... exist";



my @layers = IN->get_layers();

ok scalar( grep{ $_ eq 'crlf' } @layers ), "has other layers (in [@layers])";

close *IN;
unlink $file;

{
	use open IO => ':creat';

	ok sysopen(*IN, $file, O_RDWR), "sysopen with :creat";

	ok -e $file, "... exist";

}

eval{
	use warnings FATAL => 'layer';
	binmode *IN, ":creat";
};

like $@, qr/Too late/, "Useless use of :creat";

ok close(*IN), "close";


END{
	unlink $file if defined($file) and -e $file;
}