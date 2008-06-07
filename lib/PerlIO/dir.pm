require PerlIO::Util;
__END__
=head1 NAME

PerlIO::dir - Reads directories

=head1 SYNOPSIS

	open my $dp, '<:dir', '.';

	binmode $dp, ':encoding(cp932)'; # OK

	my @dirs = <$dp>; # added "\n" at the end of the name
	chomp @dirs; # if necessary

=head1 DESCRIPTION

C<PerlIO::dir> provides an interface to read directories.

There is an important difference between C<:dir> and Perl's C<readdir()>. This
layer B<appends a newline code>, C<\n>, to the end of the name, because
C<readline()> requires input separators. Call C<chomp()> if necesary.

You can use C<seek($dir, 0, 0)> only for C<rewinddir()>. 

	seek $dir, 0, 0; # equivalent to rewinddir()

=head1 SEE ALSO

L<perlfunc/opendir>, L<perlfunc/readdir>, L<perlfunc/rewinddir>,
L<perlfunc/closedir>.

L<PerlIO::Util>.

=head1 AUTHOR

Goro Fuji E<lt>gfuji (at) cpan.orgE<gt>

=head1 LICENCE AND COPYRIGHT

Copyright (c) 2008, Goro Fuji E<lt>gfuji (at) cpan.orgE<gt>. Some rights reserved.

This module is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=cut