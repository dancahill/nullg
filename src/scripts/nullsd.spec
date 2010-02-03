%define nullsd_bindir	/usr/bin
%define nullsd_etcdir	/etc/nullsd
%define nullsd_libdir	/usr/lib/nullsd
%define nullsd_vardir	/var/lib/nullsd

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
BuildRoot:	/tmp/nullsd-rpm
Provides:	nullgroupserver
Obsoletes:	nullgroupserver

%description
NullLogic GroupServer

%prep

%setup

%build
./configure --bindir=%{nullsd_bindir} --sysconfdir=%{nullsd_etcdir} --libdir=%{nullsd_libdir} --localstatedir=%{nullsd_vardir}
make

%install
install -d ${RPM_BUILD_ROOT}/etc/init.d
install -d ${RPM_BUILD_ROOT}%{nullsd_bindir}
install -d ${RPM_BUILD_ROOT}%{nullsd_etcdir}
install -d ${RPM_BUILD_ROOT}%{nullsd_libdir}
install -d ${RPM_BUILD_ROOT}%{nullsd_vardir}
install -m755 scripts/nullsd-init.sh ${RPM_BUILD_ROOT}/etc/init.d/nullsd
cp -a bin/* ${RPM_BUILD_ROOT}%{nullsd_bindir}
cp -a etc/* ${RPM_BUILD_ROOT}%{nullsd_etcdir}
cp -a lib/* ${RPM_BUILD_ROOT}%{nullsd_libdir}
cp -a var/* ${RPM_BUILD_ROOT}%{nullsd_vardir}

%clean
rm -rf ${RPM_BUILD_ROOT}

%post
useradd -M -r -c "NullLogic GroupServer" -d %{nullsd_vardir} -s /bin/bash nullsd 2> /dev/null || true
chown -R nullsd:nullsd %{nullsd_etcdir} %{nullsd_vardir}
chmod go-rwx %{nullsd_etcdir} %{nullsd_vardir}
/sbin/chkconfig nullsd reset

%preun
/etc/init.d/nullsd stop
/sbin/chkconfig --level 0123456 nullsd off

%files
%defattr(-,root,root)
%doc doc/COPYING doc/COPYRIGHT doc/README doc/ChangeLog doc/CodingStyle var/share/htdocs/nullsd/help/*
/etc/init.d/nullsd
%{nullsd_bindir}/*
%{nullsd_etcdir}/*
%{nullsd_libdir}/*
%{nullsd_vardir}/*

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
