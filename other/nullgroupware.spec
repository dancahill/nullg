Summary: NullLogic Groupware
Name: nullgroupware
Version: 1.2.5rc2
License: GPL
Packager: Dan Cahill <cahill@nulllogic.com>
Group: Networking/Daemons
URL: http://nullgroupware.sourceforge.net/groupware/
Source: http://prdownloads.sourceforge.net/nullgroupware/nullgroupware-%{PACKAGE_VERSION}.tar.gz
BuildRoot: /var/tmp/nullgroupware-rpmbuild
%description

NullLogic Groupware

%prep
%setup

%build
make

%install
mkdir -p $RPM_BUILD_ROOT/etc/rc.d/init.d
mkdir -p $RPM_BUILD_ROOT/usr/local/nullgroupware
cp -a distrib/* $RPM_BUILD_ROOT/usr/local/nullgroupware/
install other/rc.groupware $RPM_BUILD_ROOT/etc/rc.d/init.d/nullgroupware

%post
/usr/sbin/groupadd nullgw 1>/dev/null 2>/dev/null
/usr/sbin/useradd -c "NullLogic Groupware" -d /usr/local/nullgroupware/etc -g nullgw nullgw 1>/dev/null 2>&1
chown -R nullgw:nullgw /usr/local/nullgroupware/etc /usr/local/nullgroupware/var 1>/dev/null 2>&1
chmod go-rwx /usr/local/nullgroupware/etc /usr/local/nullgroupware/var 1>/dev/null 2>&1
/sbin/chkconfig nullgroupware reset

%preun
/etc/rc.d/init.d/nullgroupware stop
/sbin/chkconfig --level 0123456 nullgroupware off

%files
%defattr(-,root,root)
%doc COPYING COPYRIGHT distrib/README.txt Documentation/ChangeLog Documentation/CodingStyle distrib/var/htdocs/groupware/help/*
/usr/local/nullgroupware/bin/*
/usr/local/nullgroupware/cgi-bin/*
/usr/local/nullgroupware/etc/*
/usr/local/nullgroupware/lib/*
/usr/local/nullgroupware/var/*

%changelog
* Fri Feb 06 2004 Dan Cahill <cahill@nulllogic.com>
  - Release Version 1.2.5
* Mon Dec 01 2003 Dan Cahill <cahill@nulllogic.com>
  - Release Version 1.2.4
* Sat Aug 02 2003 Dan Cahill <cahill@nulllogic.com>
  - Release Version 1.2.3
* Wed May 07 2003 Dan Cahill <cahill@nulllogic.com>
  - Release Version 1.2.2
* Tue Mar 18 2003 Dan Cahill <cahill@nulllogic.com>
  - Release Version 1.2.1
* Sat Dec 28 2002 Dan Cahill <cahill@nulllogic.com>
  - Release Version 1.2.0
  - Initial 1.2.x release
