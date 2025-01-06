Name:       harbour-valuelogger2

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}

Summary:    Value Logger
Version:    1.0.21
Release:    1
License:    MIT
URL: https://github.com/monich/valuelogger
Source0:    %{name}-%{version}.tar.bz2

Requires:   qt5-qtsvg-plugin-imageformat-svg
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(sailfishapp)
BuildRequires:  pkgconfig(mlite5)
BuildRequires:  desktop-file-utils
BuildRequires:  qt5-qttools-linguist

%description
Simple application to log different data from user input, e.g. weight, blood pressure, anything

%if "%{?vendor}" == "chum"
Categories:
 - Utility
Icon: https://raw.githubusercontent.com/monich/valuelogger/master/qml/images/harbour-valuelogger2.svg
Screenshots:
- https://home.monich.net/chum/harbour-valuelogger2/screenshots/screenshot-001.png
- https://home.monich.net/chum/harbour-valuelogger2/screenshots/screenshot-002.png
- https://home.monich.net/chum/harbour-valuelogger2/screenshots/screenshot-003.png
- https://home.monich.net/chum/harbour-valuelogger2/screenshots/screenshot-004.png
- https://home.monich.net/chum/harbour-valuelogger2/screenshots/screenshot-005.png
- https://home.monich.net/chum/harbour-valuelogger2/screenshots/screenshot-006.png
- https://home.monich.net/chum/harbour-valuelogger2/screenshots/screenshot-007.png
- https://home.monich.net/chum/harbour-valuelogger2/screenshots/screenshot-008.png
- https://home.monich.net/chum/harbour-valuelogger2/screenshots/screenshot-009.png
- https://home.monich.net/chum/harbour-valuelogger2/screenshots/screenshot-010.png
Url:
  Homepage: https://openrepos.net/content/slava/value-logger
%endif

%prep
%setup -q -n %{name}-%{version}

%build

%qtc_qmake5 SPECVERSION=%{version}

%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}

%qmake5_install

desktop-file-install --delete-original \
  --dir %{buildroot}%{_datadir}/applications \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(644,root,root,755)
%attr(755,root,root) %{_bindir}
%{_datadir}/%{name}/
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
