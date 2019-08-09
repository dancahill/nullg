%define nullgw_bindir	/usr/bin
%define nullgw_etcdir	/etc/nullgw
%define nullgw_libdir	/usr/lib/nullgw
%define nullgw_vardir	/var/lib/nullgw

Name:		nullgroupware
Version:	1.3.30
Release:	1
Group:		Networking/Daemons
Packager:	Dan Cahill <http://nullgroupware.sourceforge.net/groupware/>
URL:		http://nullgroupware.sourceforge.net/groupware/
Source:		http://prdownloads.sourceforge.net/nullgroupware/nullgroupware-%{PACKAGE_VERSION}.tar.gz
Summary:	NullLogic Groupware
Vendor:		NullLogic
License:	GNU GPL Version 2
BuildRoot:	/tmp/nullgroupware-rpm
Provides:	nullgroupware
Obsoletes:	nullgroupware

%description
NullLogic Groupware

%prep

%setup

%build
./configure --bindir=%{nullgw_bindir} --sysconfdir=%{nullgw_etcdir} --libdir=%{nullgw_libdir} --localstatedir=%{nullgw_vardir}
make

%install
install -d ${RPM_BUILD_ROOT}/etc/init.d
install -d ${RPM_BUILD_ROOT}%{nullgw_bindir}
install -d ${RPM_BUILD_ROOT}%{nullgw_etcdir}
install -d ${RPM_BUILD_ROOT}%{nullgw_libdir}
install -d ${RPM_BUILD_ROOT}%{nullgw_vardir}
install -m755 scripts/nullgw-init.sh ${RPM_BUILD_ROOT}/etc/init.d/nullgroupware
cp -a distrib/bin/* ${RPM_BUILD_ROOT}%{nullgw_bindir}
cp -a distrib/etc/* ${RPM_BUILD_ROOT}%{nullgw_etcdir}
cp -a distrib/lib/* ${RPM_BUILD_ROOT}%{nullgw_libdir}
cp -a distrib/var/* ${RPM_BUILD_ROOT}%{nullgw_vardir}

%clean
rm -rf ${RPM_BUILD_ROOT}

%post
useradd -M -r -c "NullLogic Groupware" -d %{nullgw_vardir} -s /bin/bash nullgw 2> /dev/null || true
chown -R nullgw:nullgw %{nullgw_etcdir} %{nullgw_vardir}
chmod go-rwx %{nullgw_etcdir} %{nullgw_vardir}
/sbin/chkconfig nullgroupware reset

%preun
/etc/init.d/nullgroupware stop
/sbin/chkconfig --level 0123456 nullgroupware off

%files
%defattr(-,root,root)
%doc Documentation/COPYING Documentation/COPYRIGHT Documentation/README Documentation/ChangeLog Documentation/CodingStyle distrib/var/share/htdocs/groupware/help/*
/etc/init.d/nullgroupware
%{nullgw_bindir}/*
%{nullgw_etcdir}/*
%{nullgw_libdir}/*
%{nullgw_vardir}/*

%changelog
* Tue Oct 26 2004 Dan Cahill <nulllogic@users.sourceforge.net>
- Release Version 1.3.22
* Mon Oct 18 2004 Dan Cahill <nulllogic@users.sourceforge.net>
- Release Version 1.3.21
* Fri Aug 27 2004 Dan Cahill <nulllogic@users.sourceforge.net>
- Release Version 1.3.20
* Mon Aug 16 2004 Dan Cahill <nulllogic@users.sourceforge.net>
- Release Version 1.3.19
* Tue Jul 27 2004 Dan Cahill <nulllogic@users.sourceforge.net>
- Release Version 1.3.18
* Thu May 27 2004 Dan Cahill <nulllogic@users.sourceforge.net>
- Release Version 1.3.17
* Mon May 24 2004 Dan Cahill <nulllogic@users.sourceforge.net>
- Release Version 1.3.16
* Thu May 20 2004 Dan Cahill <nulllogic@users.sourceforge.net>
- Release Version 1.3.15
* Mon May 17 2004 Dan Cahill <nulllogic@users.sourceforge.net>
- Release Version 1.3.14
* Fri May 14 2004 Dan Cahill <nulllogic@users.sourceforge.net>
- Release Version 1.3.13
* Fri May 07 2004 Dan Cahill <nulllogic@users.sourceforge.net>
- Release Version 1.3.12
* Mon May 03 2004 Dan Cahill <nulllogic@users.sourceforge.net>
- Release Version 1.3.11
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
