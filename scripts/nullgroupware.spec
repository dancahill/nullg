Summary: NullLogic Groupware
Name: nullgroupware
Version: 1.3.10
Release: 1
License: GPL
Packager: Dan Cahill <nulllogic@users.sourceforge.net>
Group: Networking/Daemons
URL: http://nullgroupware.sourceforge.net/groupware/
Source: http://prdownloads.sourceforge.net/nullgroupware/nullgroupware-%{PACKAGE_VERSION}.tar.gz
BuildRoot: /var/tmp/nullgroupware-rpmbuild
%description

NullLogic Groupware

%prep
%setup

%build
./configure
make

%install
mkdir -p $RPM_BUILD_ROOT/etc/init.d
mkdir -p $RPM_BUILD_ROOT/usr/local/nullgroupware
cp -a distrib/* $RPM_BUILD_ROOT/usr/local/nullgroupware/
install scripts/rc.groupware $RPM_BUILD_ROOT/etc/init.d/nullgw

%post
/usr/sbin/groupadd nullgw 1>/dev/null 2>/dev/null
/usr/sbin/useradd -c "NullLogic Groupware" -d /usr/local/nullgroupware/etc -g nullgw nullgw 1>/dev/null 2>&1
chown -R nullgw:nullgw /usr/local/nullgroupware/etc /usr/local/nullgroupware/var 1>/dev/null 2>&1
chmod go-rwx /usr/local/nullgroupware/etc /usr/local/nullgroupware/var 1>/dev/null 2>&1
/sbin/chkconfig nullgw reset

%preun
/etc/init.d/nullgw stop
/sbin/chkconfig --level 0123456 nullgw off

%files
%defattr(-,root,root)
%doc COPYING COPYRIGHT distrib/README.txt Documentation/ChangeLog Documentation/CodingStyle distrib/var/htdocs/groupware/help/*
/etc/init.d/nullgw
/usr/local/nullgroupware/*

%changelog
* Mon Apr 26 2004 Dan Cahill <nulllogic@users.sourceforge.net>
  - Release Version 1.3.10
* Thu Apr 22 2004 Dan Cahill <nulllogic@users.sourceforge.net>
  - Release Version 1.3.9
* Mon Apr 19 2004 Dan Cahill <nulllogic@users.sourceforge.net>
  - Release Version 1.3.8
* Fri Apr 16 2004 Dan Cahill <nulllogic@users.sourceforge.net>
  - Release Version 1.3.7
* Mon Apr 12 2004 Dan Cahill <nulllogic@users.sourceforge.net>
  - Release Version 1.3.6
* Wed Apr 07 2004 Dan Cahill <nulllogic@users.sourceforge.net>
  - Release Version 1.3.5
* Sat Apr 03 2004 Dan Cahill <nulllogic@users.sourceforge.net>
  - Release Version 1.3.4
* Tue Mar 24 2004 Dan Cahill <nulllogic@users.sourceforge.net>
  - Release Version 1.3.3
* Sun Mar 21 2004 Dan Cahill <nulllogic@users.sourceforge.net>
  - Release Version 1.3.2
* Mon Mar 08 2004 Dan Cahill <nulllogic@users.sourceforge.net>
  - Release Version 1.3.1
* Thu Feb 26 2004 Dan Cahill <nulllogic@users.sourceforge.net>
  - Release Version 1.3.0
  - Initial 1.3.x release
