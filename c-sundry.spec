Name:           c-sundry
Version:        1
Release:        3
Summary:        sundry convenience headers
License:        LGPL2+
URL:            https://github.com/c-util/c-sundry
Source0:        %{name}.tar.xz
BuildRequires:  autoconf automake pkgconfig
%define debug_package %{nil}

%description
Collection of sundry convenience headers

%package        devel
Summary:        Development files for %{name}

%description    devel
The %{name}-devel package contains sundry convenience header files.

%prep
%setup -q

%build
./autogen.sh
%configure
make %{?_smp_mflags}

%install
%make_install

%files devel
%doc COPYING
%{_includedir}/c-*.h
%{_libdir}/pkgconfig/c-sundry.pc

%changelog
* Thu Jun 2 2016 <lars@uebernic.de> 1-3
- update for c-list.h

* Sun May 15 2016 <kay@redhat.com> 1-2
- update for c-syscall.h

* Sat May 14 2016 <kay@redhat.com> 1-1
- intial release
