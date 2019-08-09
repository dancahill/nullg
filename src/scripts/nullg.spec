%define nullg_bindir	/usr/bin
%define nullg_etcdir	/etc/nullg
%define nullg_libdir	/usr/lib/nullg
%define nullg_vardir	/var/lib/nullg

Name:		nullgroupserver
Version:	1.3.30
Release:	1
Group:		Networking/Daemons
Packager:	Dan Cahill <http://nullgroupware.sourceforge.net/groupserver/>
URL:		http://nullgroupware.sourceforge.net/groupserver/
Source:		http://prdownloads.sourceforge.net/nullgroupware/ngs-%{PACKAGE_VERSION}.tar.gz
Summary:	NullLogic GroupServer
Vendor:		NullLogic
License:	GNU GPL Version 2
BuildRoot:	/tmp/nullg-rpm
Provides:	nullgroupserver
Obsoletes:	nullgroupserver

%description
NullLogic GroupServer

%prep

%setup

%build
./configure --bindir=%{nullg_bindir} --sysconfdir=%{nullg_etcdir} --libdir=%{nullg_libdir} --localstatedir=%{nullg_vardir}
make

%install
install -d ${RPM_BUILD_ROOT}/etc/init.d
install -d ${RPM_BUILD_ROOT}%{nullg_bindir}
install -d ${RPM_BUILD_ROOT}%{nullg_etcdir}
install -d ${RPM_BUILD_ROOT}%{nullg_libdir}
install -d ${RPM_BUILD_ROOT}%{nullg_vardir}
install -m755 scripts/nullg-init.sh ${RPM_BUILD_ROOT}/etc/init.d/nullg
cp -a bin/* ${RPM_BUILD_ROOT}%{nullg_bindir}
cp -a etc/* ${RPM_BUILD_ROOT}%{nullg_etcdir}
cp -a lib/* ${RPM_BUILD_ROOT}%{nullg_libdir}
cp -a var/* ${RPM_BUILD_ROOT}%{nullg_vardir}

%clean
rm -rf ${RPM_BUILD_ROOT}

%post
useradd -M -r -c "NullLogic GroupServer" -d %{nullg_vardir} -s /bin/bash nullg 2> /dev/null || true
chown -R nullg:nullg %{nullg_etcdir} %{nullg_vardir}
chmod go-rwx %{nullg_etcdir} %{nullg_vardir}
/sbin/chkconfig nullg reset

%preun
/etc/init.d/nullg stop
/sbin/chkconfig --level 0123456 nullg off

%files
%defattr(-,root,root)
%doc doc/COPYING doc/COPYRIGHT doc/README doc/ChangeLog doc/CodingStyle var/share/htdocs/nullg/help/*
/etc/init.d/nullg
%{nullg_bindir}/*
%{nullg_etcdir}/*
%{nullg_libdir}/*
%{nullg_vardir}/*

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
