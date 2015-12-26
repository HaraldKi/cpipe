## $Revision: 1.3 $, $Date: 2003/07/22 08:16:29 $
## The following line is edited by my ship script to contain the true
## version I am shipping from cvs. (kir)
%define VERSION 77.66.55

Summary: counting pipe
Name: cpipe
Version: %VERSION
Release: 0
Copyright: GPL
Group: Applications/Archiving
Source: cpipe-%{VERSION}.tar.gz
URL: http://wsd.iitb.fhg.de/~kir/cpipehome/
Packager: Harald Kirsch (pifpafpuf@gmx.de)

BuildRoot: /tmp/cpipe-rpmbuild

%description
Cpipe copies its standard input to its standard output while measuring
the time it takes to read an input buffer and write an output
buffer. Statistics of average throughput and the total amount of bytes
copied are printed to the standard error output.

%prep
%setup

%build
make

%install
rm -rf $RPM_BUILD_ROOT/usr/
make install prefix=$RPM_BUILD_ROOT/usr

%post

%files
%attr(-,root,root) /usr/man/man1
%attr(-,root,root) /usr/bin
