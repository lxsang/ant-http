/*!
 * OS.js - JavaScript Cloud/Web Desktop Platform
 *
 * Copyright (c) 2011-2016, Anders Evenrud <andersevenrud@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @author  Anders Evenrud <andersevenrud@gmail.com>
 * @licence Simplified BSD License
 */
(function(Application, Window, GUI, Dialogs, Utils, API, VFS) {
  // jscs:disable validateQuoteMarks
  'use strict';

  /////////////////////////////////////////////////////////////////////////////
  // LOCALES
  /////////////////////////////////////////////////////////////////////////////

  var _Locales = {
    bg_BG : {
      'Background Type' : 'Тип на фон',
      'Image (Repeat)' : 'Изображение (повтарящо се)',
      'Image (Centered)' : 'Изображение (Центрирано)',
      'Image (Fill)' : 'Изображение (Запълващо)',
      'Image (Streched)' : 'Изображение (Разтеглено)',
      'Desktop Margin ({0}px)' : 'Размер на работен плот ({0}px)',
      'Enable Animations' : 'Разреши анимации',
      'Language (requires restart)' : 'Език (нуждае се от рестарт)',
      'Enable Sounds' : 'Включи звуци',
      'Enable Window Switcher' : 'Включи превключване на прозорци',
      'Enable Hotkeys' : 'Включи горещи клавиши',
      'Enable Icon View' : 'Включи иконен-изглед'
    },
    de_DE : {
      'Background Type' : 'Hintergrundtyp',
      'Image (Repeat)' : 'Bild (Wiederholend)',
      'Image (Centered)' : 'Bild (Zentriert)',
      'Image (Fill)' : 'Bild (Ausgefüllt)',
      'Image (Streched)' : 'Bild (Gestreckt)',
      'Desktop Margin ({0}px)' : 'Arbeitsoberflächen Margin ({0}px)',
      'Enable Animations' : 'Animationen verwenden',
      'Language (requires restart)' : 'Sprache (benötigt Neustart)',
      'Enable Sounds' : 'Aktiviere Sounds',
      'Enable Window Switcher' : 'Aktiviere Fensterwechsler',
      'Enable Hotkeys' : 'Aktiviere Hotkeys',
      'Enable Icon View' : 'Aktiviere Icon-Ansicht',
    },
    es_ES : {
      'Background Type' : 'Tipo de fondo',
      'Image (Repeat)' : 'Imagen (Repetir)',
      'Image (Centered)' : 'Imagen (Centrada)',
      'Image (Fill)' : 'Imagen (Estirar)',
      'Image (Streched)' : 'Imagen (Ajustar)',
      'Desktop Margin ({0}px)' : 'Margen del escritorio ({0}px)',
      'Enable Animations' : 'Habilitar animaciones',
      'Language (requires restart)' : 'Idioma (requiere reiniciar)',
      'Enable Sounds' : 'Activar sonidos',
      'Enable Window Switcher' : 'Activar el alternador de ventanas',
      'Enable Hotkeys' : 'Activar Hotkeys',
      'Enable Icon View' : 'Activar la vista de icono',
    },
    ar_DZ : {
      'Background Type' : 'نوع الخلفية',
      'Image (Repeat)' : 'صورة (إعادة)',
      'Image (Centered)' : 'صورة (وسط)',
      'Image (Fill)' : 'صورة (ملئ)',
      'Image (Streched)' : 'صورة (تمدد)',
      'Desktop Margin ({0}px)' : 'هوامش المكتب ({0}px)',
      'Enable Animations' : 'تفعيل الحركة',
      'Language (requires restart)' : 'اللغة (تتطب إعادة التشغيل)',
      'Enable Sounds' : 'تفعيل الأصوات',
      'Enable Window Switcher' : 'تفعيل محول النوافذ',
      'Enable Hotkeys' : 'تفعيل إختصارات لوحة المفاتيح',
      'Enable Icon View' : 'تفعيل مظهر الأيقونات',
      'Remove shortcut' : 'حذف الإختصار',
      'File View': 'خصائص الملفات',
      'Show Hidden Files': 'إظهار الملفات المخفية',
      'Show File Extensions': 'إظهار لواحق الملفات',
      'File View Options': 'خيارات إظهار الملفات',
      'Invert Text Color' : 'عكس لون الخط',
      'Icon View' : 'إظهار الأيقونات',
      'Installed Packages' : 'حزم مثبتة',
      'App Store' : 'متجر التطبيقات',
      'Regenerate metadata' : 'إعادة توليد المعلومات',
      'Install from zip' : 'تثبيت من ملف مضغوط',
      'Install selected' : 'تثبيت المختار',
      'Enable TouchMenu' : 'تفعيل قائمة اللمس'
    },
    fr_FR : {
      'Background Type' : 'Type de fond d\'écran',
      'Image (Repeat)' : 'Image (Répéter)',
      'Image (Centered)' : 'Image (Centrer)',
      'Image (Fill)' : 'Image (Remplir)',
      'Image (Streched)' : 'Image (Étiré)',
      'Desktop Margin ({0}px)' : 'Marge du bureau ({0}px)',
      'Desktop Corner Snapping ({0}px)' : 'Délimitation des coins du bureau ({0}px)',
      'Window Snapping ({0}px)' : 'Accrochage des fenêtres ({0}px)',
      'Enable Animations' : 'Activer les animations',
      'Language (requires restart)' : 'Langue (redémarrage requis)',
      'Enable Sounds' : 'Activer la musique',
      'Enable Window Switcher' : 'Activer Window Switcher',
      'Enable Hotkeys' : 'Activer les raccourcis clavier',
      'Enable Icon View' : 'Activer l\'affichage des icônes sur le bureau',
      'Remove shortcut' : 'Supprimer le raccourci',
      'File View': 'Options des fichiers',
      'Show Hidden Files': 'Montrer les fichiers cachés',
      'Show File Extensions': 'Montrer les extensions de fichiers',
      'File View Options': 'Options d\'affichage des fichier',
      'Invert Text Color' : 'Inverser la couleur du texte',
      'Icon View' : 'Affichage des icônes',
      'Installed Packages' : 'Paquets installés',
      'App Store' : 'Magasin d\'applications',
      'Regenerate metadata' : 'Régénérer les métadonnées',
      'Install from zip' : 'Installer à partir du fichier zip',
      'Install selected' : 'Installer la sélection',
      'Enable TouchMenu' : 'Activer le TouchMenu'
    },
    it_IT : {
      'Background Type' : 'Tipo di sfondo',
      'Image (Repeat)' : 'Immagine (Ripeti)',
      'Image (Centered)' : 'Immagine (Centrata)',
      'Image (Fill)' : 'Immagine (Riempi)',
      'Image (Streched)' : 'Immagine (Distorci)',
      'Desktop Margin ({0}px)' : 'Margini Scrivania ({0}px)',
      'Enable Animations' : 'Abilita animazioni',
      'Language (requires restart)' : 'Lingua (necessita riavvio)',
      'Enable Sounds' : 'Abilita Suoni',
      'Enable Window Switcher' : 'Abilita Cambia-Finestre',
      'Enable Hotkeys' : 'Abilita Scorciatoie da tastiera',
      'Enable Icon View' : 'Abilita Visualizzazione ad icona',
      'Remove shortcut' : 'Rimuovi scorciatoia',
      'File View': 'Visualizza file',
      'Show Hidden Files': 'Mostra file nascosti',
      'Show File Extensions': 'Mostra estenzioni dei file',
      'File View Options': 'Opzioni visualizza file',
      'Invert Text Color' : 'Inverti colore testi',
      'Icon View' : 'Visualizzazione ad icone',
      'Installed Packages' : 'Installa pacchetti',
      'App Store' : 'Negozio applicazioni',
      'Application' : 'Applicazione',
      'Scope' : 'Scope (namespace)',
      'Regenerate metadata' : 'Rigenerazione metadata',
      'Install from zip' : 'Installa da zip',
      'Install selected' : 'Installa selezionato',
      'Enable TouchMenu' : 'Abilita TouchMenu'
    },
    ko_KR : {
      'Background Type' : '바탕화면 타입',
      'Image (Repeat)' : '이미지 (반복)',
      'Image (Centered)' : '이미지 (가운데)',
      'Image (Fill)' : '이미지 (채우기)',
      'Image (Streched)' : '이미지 (늘이기)',
      'Desktop Margin ({0}px)' : '데스크탑 여백 ({0}px)',
      'Enable Animations' : '애니메이션 효과 켜기',
      'Language (requires restart)' : '언어 (재시작 필요)',
      'Enable Sounds' : '사운드 켜기',
      'Enable Window Switcher' : '윈도우 전환 활성',
      'Enable Hotkeys' : '단축키 활성',
      'Enable Icon View' : '아이콘 보이기',
      'Desktop Corner Snapping ({0}px)' : '바탕화면 가장자리에 붙이기 ({0}px)',
      'Window Snapping ({0}px)' : '창 가장자리에 붙이기 ({0}px)',
      'File View': '파일보기',
      'Show Hidden Files': '숨긴 파일 보이기',
      'Show File Extensions': '파일 확장자 보이기',
      'File View Options': '파일보기 옵션',
      'Invert Text Color' : '텍스트 색상 반전',
      'Icon View' : '아이콘 보기',
      'Installed Packages' : '설치된 패키지',
      'App Store' : '앱스토어',
      'Regenerate metadata' : '메타데이터 재생성',
      'Install from zip' : 'zip 파일로부터 설치하기',
      'Install selected' : '선택된 항목 설치',
      'Enable TouchMenu' : '터치메뉴 활성화',
      'Search Options' : '검색 옵션',
      'Enable Application Search' : '어플리케이션 검색 활성화',
      'Enable File Search' : '파일 검색 활성화'
    },
    nl_NL : {
      'Background Type' : 'Achtergrond type',
      'Image (Repeat)' : 'Afbeelding (Herhalend)',
      'Image (Centered)' : 'Afbeelding (Gecentreerd)',
      'Image (Fill)' : 'Afbeelding (Passend)',
      'Image (Streched)' : 'Afbeelding (Uitrekken)',
      'Desktop Margin ({0}px)' : 'Achtergrondmarge ({0}px)',
      'Enable Animations' : 'Animaties gebruiken',
      'Language (requires restart)' : 'Taal (Herstarten vereist)',
      'Enable Sounds' : 'Activeer Geluiden',
      'Enable Window Switcher' : 'Activeer Venster Wisselaar',
      'Enable Hotkeys' : 'Activeer Hotkeys',
      'Enable Icon View' : 'Activeer Iconen-weergave'
    },
    no_NO : {
      'Background Type' : 'Bakgrunn type',
      'Image (Repeat)' : 'Bilde (Gjenta)',
      'Image (Centered)' : 'Bilde (Sentrert)',
      'Image (Fill)' : 'Bilde (Fyll)',
      'Image (Streched)' : 'Bilde (Strekk)',
      'Desktop Margin ({0}px)' : 'Skrivebord Margin ({0}px)',
      'Enable Animations' : 'Bruk animasjoner',
      'Language (requires restart)' : 'Språk (krever omstart)',
      'Enable Sounds' : 'Skru på lyder',
      'Enable Window Switcher' : 'Skru på Vindu-bytter',
      'Enable Hotkeys' : 'Skru på Hurtigtaster',
      'Enable Icon View' : 'Skru på Ikonvisning',
      'Remove shortcut' : 'Fjern snarvei',
      'Search path \'{0}\' is already handled by another entry': 'Søkestien \'{0}\' er allrede håndtert av en annen oppføring'
    },
    pl_PL : {
      'Background Type' : 'Typ Tła',
      'Image (Repeat)' : 'Powtarzający się',
      'Image (Centered)' : 'Wycentrowany',
      'Image (Fill)' : 'Wypełniony',
      'Image (Streched)' : 'Rozciągnięty',
      'Desktop Margin ({0}px)' : 'Margines Pulpitu ({0}px)',
      'Desktop Corner Snapping ({0}px)' : 'Przyciąganie do Narożników Pulpitu ({0}px)',
      'Window Snapping ({0}px)' : 'Przyciąganie do Okien ({0}px)',
      'Enable Animations' : 'Włączone Animacje',
      'Icon View' : 'Widok Ikon',
      'Language (requires restart)' : 'Język (zmiana wymaga restartu)',
      'Enable Sounds' : 'Włączone Dźwięki',
      'Enable TouchMenu' : 'Włączone Menu Dotykowe',
      'Enable Window Switcher' : 'Właczony Zmieniacz Okien',
      'Enable Hotkeys' : 'Włączone Skróty Klawiaturowe',
      'Enable Icon View' : 'Włączone Pokazywanie Ikon',
      'Remove shortcut' : 'Usuwanie skrótu',
      'File View': 'Widok Plików',
      'Show Hidden Files': 'Pokazuj Ukryte Pliki',
      'Show File Extensions': 'Pokazuj Rozszerzenia Plików',
      'File View Options': 'Opcje Widoku Plików',
      'Invert Text Color' : 'Odwróć Kolor Tekstu',
      'Installed Packages' : 'Zainstalowane Pakiety',
      'App Store' : 'Sklep App',
      'Regenerate metadata' : 'Zregeneruj metadane',
      'Install from zip' : 'Zainstaluj z pliku zip',
      'Install selected' : 'Zainstaluj wybrane'
    },
    ru_RU : {
      'Background Type' : 'Тип фона',
      'Image (Repeat)' : 'Изображение (повторяющееся)',
      'Image (Centered)' : 'Изображение (по центру)',
      'Image (Fill)' : 'Изображение (заполнить)',
      'Image (Streched)' : 'Изображение (растянуть)',
      'Desktop Margin ({0}px)' : 'Отступ рабочего стола ({0}px)',
      'Enable Animations' : 'Использовать анимацию',
      'Enable TouchMenu' : 'Крупное меню',
      'Language (requires restart)' : 'Язык (необходим перезапуск)',
      'Enable Sounds' : 'Включить звук',
      'Enable Window Switcher' : 'Включить растягивание окон',
      'Enable Hotkeys' : 'Включить горячии клавиши',
      'Enable Icon View' : 'Включить ярлыки',
      'Icon View' : 'Ярлыки рабочего стола',
      'Invert Text Color' : 'Обратить цвет текста'
    },
    sk_SK : {
      'Background Type' : 'Typ pozadia',
      'Image (Repeat)' : 'Dlaždice',
      'Image (Centered)' : 'Na stred',
      'Image (Fill)' : 'Vyplniť',
      'Image (Streched)' : 'Roztiahnutý',
      'Desktop Margin ({0}px)' : 'Hranice pracovnej plochy ({0}px)',
      'Enable Animations' : 'Povoliť animácie',
      'Language (requires restart)' : 'Jazyk (vyžaduje reštart)',
      'Enable Sounds' : 'Povoliť zvuky',
      'Enable Window Switcher' : 'Povoliť Prepínač Okien',
      'Enable Hotkeys' : 'Klávesové skratky',
      'Enable Icon View' : 'Ikony na ploche',
      'Remove shortcut' : 'Odstrániť skratku'
    },
    tr_TR : {
      'Background Type' : 'arkaplan türü',
      'Image (Repeat)' : 'resim (tekrarla)',
      'Image (Centered)' : 'resm(ortala)',
      'Image (Fill)' : 'resm (kapla/doldur)',
      'Image (Streched)' : 'resm (uzat)',
      'Desktop Margin ({0}px)' : 'masaüstü kenar ({0}px)',
      'Enable Animations' : 'animasyonlar etkin',
      'Language (requires restart)' : 'Dil(yeniden başlatma gerektirir)',
      'Enable Sounds' : 'Müzik etkin',
      'Enable Window Switcher' : 'Ekran(pencere) değiştirme etkin',
      'Enable Hotkeys' : 'kısayol tuşları etkin',
      'Enable Icon View' : 'icon görünümü etkin',
      'Remove shortcut' : 'kısayolları kaldır'
    },
    vi_VN : {
      'Background Type' : 'Kiểu nền',
      'Image (Repeat)' : 'Lặp lại',
      'Image (Centered)' : 'Căn giữa',
      'Image (Fill)' : 'Lấp đầy',
      'Image (Streched)' : 'Trải dài',
      'Desktop Margin ({0}px)' : 'Phần biên màn hình ({0}px)',
      'Enable Animations' : 'Bật hiệu ứng',
      'Language (requires restart)' : 'Ngôn ngữ (cần khởi động lại)',
      'Enable Sounds' : 'Bật âm thanh',
      'Enable Window Switcher' : 'Bật chuyển đổi cửa sổ',
      'Enable Hotkeys' : 'Bật phím nóng',
      'Enable Icon View' : 'Hiện biểu tượng',
      'Remove shortcut' : 'Xóa lối tắt',
      'File View': 'Quản lí tệp',
      'Show Hidden Files': 'Hiện tập tin ẩn',
      'Show File Extensions': 'Hiện đuôi tập tin',
      'File View Options': 'Cài đặt quản lí tệp',
      'Icon View' : 'Biểu tượng',
      'Installed Packages' : 'Các phần mềm đã cài',
      'App Store' : 'Chợ ứng dụng',
      'Regenerate metadata' : 'Làm mới metadata',
      'Install from zip' : 'Cài từ file zip',
      'Install selected' : 'Cài mục đã chọn',
      'Enable TouchMenu' : 'Bật Menu cảm ứng',
      'Invert Text Color' : 'Đảo màu chữ',
      'Search Options' : 'Cài đặt tìm kiếm',
      'Enable Application Search' : 'Cho phép tìm kiếm phần mềm',
      'Enable File Search' : 'Cho phép tìm kiếm tập tin',
      'Search path \'{0}\' is already handled by another entry': 'Đường dẫn tìm kiếm \'{0}\' đã bị xử lý bởi mục khác'
    }
  };

  function _() {
    var args = Array.prototype.slice.call(arguments, 0);
    args.unshift(_Locales);
    return API.__.apply(this, args);
  }

  /////////////////////////////////////////////////////////////////////////////
  // EXPORTS
  /////////////////////////////////////////////////////////////////////////////

  OSjs.Applications = OSjs.Applications || {};
  OSjs.Applications.ApplicationSettings = OSjs.Applications.ApplicationSettings || {};
  OSjs.Applications.ApplicationSettings._ = _;

})(OSjs.Core.Application, OSjs.Core.Window, OSjs.GUI, OSjs.Dialogs, OSjs.Utils, OSjs.API, OSjs.VFS);

/*!
 * OS.js - JavaScript Cloud/Web Desktop Platform
 *
 * Copyright (c) 2011-2016, Anders Evenrud <andersevenrud@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @author  Anders Evenrud <andersevenrud@gmail.com>
 * @licence Simplified BSD License
 */
(function(Application, Window, Utils, API, VFS, GUI) {
  'use strict';

  var DEFAULT_GROUP = 'misc';

  var _groups = {
    personal: {
      label: 'LBL_PERSONAL'
    },
    system: {
      label: 'LBL_SYSTEM'
    },
    user: {
      label: 'LBL_USER'
    },
    misc: {
      label: 'LBL_OTHER'
    }
  };

  var categoryMap = {
    'theme': 'Theme',
    'desktop': 'Desktop',
    'panel': 'Panel',
    'user': 'User',
    'fileview': 'VFS',
    'search': 'Search'
  };

  /////////////////////////////////////////////////////////////////////////////
  // WINDOWS
  /////////////////////////////////////////////////////////////////////////////

  function ApplicationSettingsWindow(app, metadata, scheme, initialCategory) {
    Window.apply(this, ['ApplicationSettingsWindow', {
      icon: metadata.icon,
      title: metadata.name,
      width: 500,
      height: 450,
      allow_resize: true
    }, app, scheme]);

    this.initialCategory = initialCategory;
  }

  ApplicationSettingsWindow.prototype = Object.create(Window.prototype);
  ApplicationSettingsWindow.constructor = Window.prototype;

  ApplicationSettingsWindow.prototype.init = function(wmRef, app, scheme) {
    var self = this;
    var root = Window.prototype.init.apply(this, arguments);
    var wm = OSjs.Core.getWindowManager();
    var _ = OSjs.Applications.ApplicationSettings._;

    // Load and render `scheme.html` file
    scheme.render(this, 'SettingsWindow', root, null, null, {_: _});

    this._find('ButtonOK').son('click', this, this.onButtonOK);
    this._find('ButtonCancel').son('click', this, this.onButtonCancel);

    // Adds all groups and their respective entries
    var container = document.createElement('div');
    container.className = 'ListView gui-generic-zebra-container';

    var containers = {};
    var tmpcontent = document.createDocumentFragment();

    Object.keys(_groups).forEach(function(k) {
      var c = document.createElement('ul');
      var h = document.createElement('span');
      var d = document.createElement('div');

      d.className = 'gui-generic-double-padded';
      h.appendChild(document.createTextNode(_(_groups[k].label)));

      containers[k] = c;

      d.appendChild(h);
      d.appendChild(c);
      container.appendChild(d);
    });

    app.modules.forEach(function(m) {
      if ( containers[m.group] ) {
        var i = document.createElement('img');
        i.setAttribute('src', API.getIcon(m.icon, '32x32'));
        i.setAttribute('title', m.name);

        var s = document.createElement('span');
        s.appendChild(document.createTextNode(_(m.label || m.name)));

        var c = document.createElement('li');
        c.className = 'gui-generic-hoverable';
        c.setAttribute('data-module', String(m.name));
        c.appendChild(i);
        c.appendChild(s);

        containers[m.group].appendChild(c);

        root.querySelector('[data-module="' + m.name +  '"]').className  = 'gui-generic-padded';

        var settings = Utils.cloneObject(wm.getSettings());
        m.render(self, scheme, tmpcontent, settings, wm);
        m.update(self, scheme, settings, wm);
        m._inited = true;
      }
    });

    Object.keys(containers).forEach(function(k) {
      if ( !containers[k].children.length ) {
        containers[k].parentNode.style.display = 'none';
      }
    });

    Utils.$bind(container, 'click', function(ev) {
      var t = ev.isTrusted ? ev.target : (ev.relatedTarget || ev.target);
      if ( t && t.tagName === 'LI' && t.hasAttribute('data-module') ) {
        ev.preventDefault();
        var m = t.getAttribute('data-module');
        self.onModuleSelect(m);
      }
    }, true);

    root.querySelector('[data-id="ContainerSelection"]').appendChild(container);

    containers = {};
    tmpcontent = null;

    if ( this.initialCategory ) {
      this.onExternalAttention(this.initialCategory);
    }

    return root;
  };

  ApplicationSettingsWindow.prototype.destroy = function() {
    // This is where you remove objects, dom elements etc attached to your
    // instance. You can remove this if not used.
    if ( Window.prototype.destroy.apply(this, arguments) ) {
      this.currentModule = null;

      return true;
    }
    return false;
  };

  ApplicationSettingsWindow.prototype.onModuleSelect = function(name) {
    var _ = OSjs.Applications.ApplicationSettings._;
    var wm = OSjs.Core.getWindowManager();
    var root = this._$element;
    var self = this;

    function _d(d) {
      root.querySelector('[data-id="ContainerSelection"]').style.display = d ? 'block' : 'none';
      root.querySelector('[data-id="ContainerContent"]').style.display = d ? 'none' : 'block';
      root.querySelector('[data-id="ContainerButtons"]').style.display = d ? 'none' : 'block';
    }

    root.querySelectorAll('div[data-module]').forEach(function(mod) {
      mod.style.display = 'none';
    });

    _d(true);

    this._setTitle(null);

    var found;
    if ( name ) {
      this._app.modules.forEach(function(m) {
        if ( !found && m.name === name ) {
          found = m;
        }
      });
    }

    if ( found ) {
      var mod = root.querySelector('div[data-module="' + found.name +  '"]');
      if ( mod ) {
        mod.style.display = 'block';
        var settings = Utils.cloneObject(wm.getSettings());
        found.update(this, this._scheme, settings, wm, true);

        _d(false);
        this._setTitle(_(found.name), true);

        if ( found.button === false ) {
          this._find('ButtonOK').hide();
        } else {
          this._find('ButtonOK').show();
        }
      }
    } else {
      if ( !name ) { // Resets values to original (or current)
        var settings = Utils.cloneObject(wm.getSettings());
        this._app.modules.forEach(function(m) {
          if ( m._inited ) {
            m.update(self, self._scheme, settings, wm);
          }
        });
      }
    }

    this._app.setModule(found);
  };

  ApplicationSettingsWindow.prototype.onButtonOK = function() {
    var self = this;
    var settings = {};
    var wm = OSjs.Core.getWindowManager();
    var saves = [];

    this._app.modules.forEach(function(m) {
      if ( m._inited ) {
        var res = m.save(self, self._scheme, settings, wm);
        if ( typeof res === 'function' ) {
          saves.push(res);
        }
      }
    });

    this._toggleLoading(true);
    this._app.saveSettings(settings, saves, function() {
      self._toggleLoading(false);
    });
  };

  ApplicationSettingsWindow.prototype.onButtonCancel = function() {
    this.onModuleSelect(null);
  };

  ApplicationSettingsWindow.prototype.onExternalAttention = function(cat) {
    this.onModuleSelect(categoryMap[cat] || cat);
    this._focus();
  };

  /////////////////////////////////////////////////////////////////////////////
  // APPLICATION
  /////////////////////////////////////////////////////////////////////////////

  function ApplicationSettings(args, metadata) {
    Application.apply(this, ['ApplicationSettings', args, metadata]);

    var self = this;
    var registered = OSjs.Applications.ApplicationSettings.Modules;

    this.watches = {};
    this.currentModule = null;

    this.modules = Object.keys(registered).map(function(name) {
      var opts = Utils.argumentDefaults(registered[name], {
        _inited: false,
        name: name,
        group: DEFAULT_GROUP,
        icon: 'status/error.png',
        init: function() {},
        update: function() {},
        render: function() {},
        save: function() {}
      });

      if ( Object.keys(_groups).indexOf(opts.group) === -1 ) {
        opts.group = DEFAULT_GROUP;
      }

      Object.keys(opts).forEach(function(k) {
        if ( typeof opts[k] === 'function' ) {
          opts[k] = opts[k].bind(opts);
        }
      });

      return opts;
    });

    this.modules.forEach(function(m) {
      m.init(self);

      if ( m.watch && m.watch instanceof Array ) {
        m.watch.forEach(function(w) {
          self.watches[m.name] = OSjs.Core.getSettingsManager().watch(w, function() {
            var win = self._getMainWindow();
            if ( m && win ) {
              if ( self.currentModule && self.currentModule.name === m.name ) {
                win.onModuleSelect(m.name);
              }
            }
          });
        });
      }
    });
  }

  ApplicationSettings.prototype = Object.create(Application.prototype);
  ApplicationSettings.constructor = Application;

  ApplicationSettings.prototype.destroy = function() {
    // This is where you remove objects, dom elements etc attached to your
    // instance. You can remove this if not used.
    if ( Application.prototype.destroy.apply(this, arguments) ) {

      var self = this;
      Object.keys(this.watches).forEach(function(k) {
        OSjs.Core.getSettingsManager().unwatch(self.watches[k]);
      });
      this.watches = {};

      return true;
    }
    return false;
  };

  ApplicationSettings.prototype.init = function(settings, metadata, scheme) {
    Application.prototype.init.apply(this, arguments);

    var category = this._getArgument('category') || settings.category;
    var win = this._addWindow(new ApplicationSettingsWindow(this, metadata, scheme, category));

    this._on('attention', function(args) {
      if ( win && args.category ) {
        win.onExternalAttention(args.category);
      }
    });
  };

  ApplicationSettings.prototype.saveSettings = function(settings, saves, cb) {
    var wm = OSjs.Core.getWindowManager();
    wm.applySettings(settings, false, function() {
      Utils.asyncs(saves, function(iter, idx, next) {
        iter(next);
      }, cb);
    }, false);
  };

  ApplicationSettings.prototype.setModule = function(m) {
    this.currentModule = m;
  };

  /////////////////////////////////////////////////////////////////////////////
  // EXPORTS
  /////////////////////////////////////////////////////////////////////////////

  OSjs.Applications = OSjs.Applications || {};
  OSjs.Applications.ApplicationSettings = OSjs.Applications.ApplicationSettings || {};
  OSjs.Applications.ApplicationSettings.Class = Object.seal(ApplicationSettings);
  OSjs.Applications.ApplicationSettings.Modules = OSjs.Applications.ApplicationSettings.Modules || {};

})(OSjs.Core.Application, OSjs.Core.Window, OSjs.Utils, OSjs.API, OSjs.VFS, OSjs.GUI);

/*!
 * OS.js - JavaScript Cloud/Web Desktop Platform
 *
 * Copyright (c) 2011-2016, Anders Evenrud <andersevenrud@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @author  Anders Evenrud <andersevenrud@gmail.com>
 * @licence Simplified BSD License
 */
(function(Application, Window, Utils, API, VFS, GUI) {
  'use strict';

  /////////////////////////////////////////////////////////////////////////////
  // MODULE
  /////////////////////////////////////////////////////////////////////////////

  var module = {
    group: 'personal',
    name: 'Theme',
    label: 'LBL_THEME',
    icon: 'apps/background.png',
    watch: ['CoreWM'],

    init: function() {
    },

    update: function(win, scheme, settings, wm) {
      win._find('BackgroundImage').set('value', settings.wallpaper);
      win._find('BackgroundColor').set('value', settings.backgroundColor);
      win._find('FontName').set('value', settings.fontFamily);

      win._find('StyleThemeName').set('value', settings.styleTheme);
      win._find('IconThemeName').set('value', settings.iconTheme);

      win._find('EnableTouchMenu').set('value', settings.useTouchMenu);

      win._find('BackgroundStyle').set('value', settings.background);
      win._find('BackgroundImage').set('value', settings.wallpaper);
      win._find('BackgroundColor').set('value', settings.backgroundColor);
    },

    render: function(win, scheme, root, settings, wm) {
      var _ = OSjs.Applications.ApplicationSettings._;

      function _createDialog(n, a, done) {
        win._toggleDisabled(true);
        API.createDialog(n, a, function(ev, button, result) {
          win._toggleDisabled(false);
          if ( button === 'ok' && result ) {
            done(result);
          }
        }, win);
      }

      win._find('StyleThemeName').add(wm.getStyleThemes().map(function(t) {
        return {label: t.title, value: t.name};
      }));

      win._find('IconThemeName').add((function(tmp) {
        return Object.keys(tmp).map(function(t) {
          return {label: tmp[t], value: t};
        });
      })(wm.getIconThemes()));

      win._find('BackgroundImage').on('open', function(ev) {
        _createDialog('File', {
          mime: ['^image'],
          file: new VFS.File(ev.detail)
        }, function(result) {
          win._find('BackgroundImage').set('value', result.path);
        });
      });

      win._find('BackgroundColor').on('open', function(ev) {
        _createDialog('Color', {
          color: ev.detail
        }, function(result) {
          win._find('BackgroundColor').set('value', result.hex);
        }, win);
      });

      win._find('FontName').on('click', function() {
        _createDialog('Font', {
          fontName: settings.fontFamily,
          fontSize: -1
        }, function(result) {
          win._find('FontName').set('value', result.fontName);
        }, win);
      });

      win._find('BackgroundStyle').add([
        {value: 'image',        label: API._('LBL_IMAGE')},
        {value: 'image-repeat', label: _('Image (Repeat)')},
        {value: 'image-center', label: _('Image (Centered)')},
        {value: 'image-fill',   label: _('Image (Fill)')},
        {value: 'image-strech', label: _('Image (Streched)')},
        {value: 'color',        label: API._('LBL_COLOR')}
      ]);
    },

    save: function(win, scheme, settings, wm) {
      settings.styleTheme = win._find('StyleThemeName').get('value');
      settings.iconTheme = win._find('IconThemeName').get('value');
      settings.useTouchMenu = win._find('EnableTouchMenu').get('value');
      settings.wallpaper = win._find('BackgroundImage').get('value');
      settings.backgroundColor = win._find('BackgroundColor').get('value');
      settings.background = win._find('BackgroundStyle').get('value');
      settings.fontFamily = win._find('FontName').get('value');
    }
  };

  /////////////////////////////////////////////////////////////////////////////
  // EXPORTS
  /////////////////////////////////////////////////////////////////////////////

  OSjs.Applications = OSjs.Applications || {};
  OSjs.Applications.ApplicationSettings = OSjs.Applications.ApplicationSettings || {};
  OSjs.Applications.ApplicationSettings.Modules = OSjs.Applications.ApplicationSettings.Modules || {};
  OSjs.Applications.ApplicationSettings.Modules.Theme = module;

})(OSjs.Core.Application, OSjs.Core.Window, OSjs.Utils, OSjs.API, OSjs.VFS, OSjs.GUI);

/*!
 * OS.js - JavaScript Cloud/Web Desktop Platform
 *
 * Copyright (c) 2011-2016, Anders Evenrud <andersevenrud@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @author  Anders Evenrud <andersevenrud@gmail.com>
 * @licence Simplified BSD License
 */
(function(Application, Window, Utils, API, VFS, GUI) {
  'use strict';

  function updateLabel(win, lbl, value) {
    var _ = OSjs.Applications.ApplicationSettings._;

    var map = {
      DesktopMargin: 'Desktop Margin ({0}px)',
      CornerSnapping: 'Desktop Corner Snapping ({0}px)',
      WindowSnapping: 'Window Snapping ({0}px)'
    };

    var label = Utils.format(_(map[lbl]), value);
    win._find(lbl + 'Label').set('value', label);
  }

  /////////////////////////////////////////////////////////////////////////////
  // MODULE
  /////////////////////////////////////////////////////////////////////////////

  var module = {
    group: 'personal',
    name: 'Desktop',
    label: 'LBL_DESKTOP',
    icon: 'devices/display.png',
    watch: ['CoreWM'],

    init: function(app) {
    },

    update: function(win, scheme, settings, wm) {
      win._find('EnableAnimations').set('value', settings.animations);
      win._find('EnableTouchMenu').set('value', settings.useTouchMenu);

      win._find('EnableWindowSwitcher').set('value', settings.enableSwitcher);

      win._find('DesktopMargin').set('value', settings.desktopMargin);
      win._find('CornerSnapping').set('value', settings.windowCornerSnap);
      win._find('WindowSnapping').set('value', settings.windowSnap);

      updateLabel(win, 'DesktopMargin', settings.desktopMargin);
      updateLabel(win, 'CornerSnapping', settings.windowCornerSnap);
      updateLabel(win, 'WindowSnapping', settings.windowSnap);
    },

    render: function(win, scheme, root, settings, wm) {
      win._find('DesktopMargin').on('change', function(ev) {
        updateLabel(win, 'DesktopMargin', ev.detail);
      });
      win._find('CornerSnapping').on('change', function(ev) {
        updateLabel(win, 'CornerSnapping', ev.detail);
      });
      win._find('WindowSnapping').on('change', function(ev) {
        updateLabel(win, 'WindowSnapping', ev.detail);
      });

      win._find('EnableIconView').set('value', settings.enableIconView);
      win._find('EnableIconViewInvert').set('value', settings.invertIconViewColor);
    },

    save: function(win, scheme, settings, wm) {
      settings.animations = win._find('EnableAnimations').get('value');
      settings.useTouchMenu = win._find('EnableTouchMenu').get('value');
      settings.enableSwitcher = win._find('EnableWindowSwitcher').get('value');
      settings.desktopMargin = win._find('DesktopMargin').get('value');
      settings.windowCornerSnap = win._find('CornerSnapping').get('value');
      settings.windowSnap = win._find('WindowSnapping').get('value');
      settings.enableIconView = win._find('EnableIconView').get('value');
      settings.invertIconViewColor = win._find('EnableIconViewInvert').get('value');
    }
  };

  /////////////////////////////////////////////////////////////////////////////
  // EXPORTS
  /////////////////////////////////////////////////////////////////////////////

  OSjs.Applications = OSjs.Applications || {};
  OSjs.Applications.ApplicationSettings = OSjs.Applications.ApplicationSettings || {};
  OSjs.Applications.ApplicationSettings.Modules = OSjs.Applications.ApplicationSettings.Modules || {};
  OSjs.Applications.ApplicationSettings.Modules.Desktop = module;

})(OSjs.Core.Application, OSjs.Core.Window, OSjs.Utils, OSjs.API, OSjs.VFS, OSjs.GUI);

/*!
 * OS.js - JavaScript Cloud/Web Search Platform
 *
 * Copyright (c) 2011-2016, Anders Evenrud <andersevenrud@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @author  Anders Evenrud <andersevenrud@gmail.com>
 * @licence Simplified BSD License
 */
(function(Application, Window, Utils, API, VFS, GUI) {
  'use strict';

  /////////////////////////////////////////////////////////////////////////////
  // MODULE
  /////////////////////////////////////////////////////////////////////////////

  var module = {
    group: 'system',
    name: 'Search',
    label: 'LBL_SEARCH',
    icon: 'actions/search.png',

    init: function() {
    },

    update: function(win, scheme, settings, wm) {

      var sm = OSjs.Core.getSettingsManager();
      var searchOptions = Utils.cloneObject(sm.get('SearchEngine') || {});

      win._find('SearchEnableApplications').set('value', searchOptions.applications === true);
      win._find('SearchEnableFiles').set('value', searchOptions.files === true);

      var view = win._find('SearchPaths').clear();
      view.set('columns', [
        {label: 'Path'}
      ]);

      var list = (searchOptions.paths || []).map(function(l) {
        return {
          value: l,
          id: l,
          columns: [
            {label: l}
          ]
        };
      });

      view.add(list);
    },

    render: function(win, scheme, root, settings, wm) {
      function openAddDialog() {
        win._toggleDisabled(true);

        API.createDialog('File', {
          select: 'dir',
          mfilter: [
            function(m) {
              return m.module.searchable === true;
            }
          ]
        }, function(ev, button, result) {
          win._toggleDisabled(false);
          if ( button === 'ok' && result ) {
            win._find('SearchPaths').add([{
              value: result.path,
              id: result.path,
              columns: [
                {label: result.path}
              ]
            }]);
          }
        }, win);
      }

      function removeSelected() {
        var view = win._find('SearchPaths');
        var current = view.get('value') || [];
        current.forEach(function(c) {
          view.remove(c.index);
        });
      }

      win._find('SearchAdd').on('click', openAddDialog);
      win._find('SearchRemove').on('click', removeSelected);
    },

    save: function(win, scheme, settings, wm) {
      var _ = OSjs.Applications.ApplicationSettings._;
      var tmpPaths = win._find('SearchPaths').get('entry', null, null, true).sort();
      var paths = [];

      function isChildOf(tp) {
        var result = false;
        paths.forEach(function(p) {
          if ( !result ) {
            result = tp.substr(0, p.length) === p;
          }
        });
        return result;
      }

      tmpPaths.forEach(function(tp) {
        var c = isChildOf(tp);
        if ( c ) {
          wm.notification({
            title: API._('LBL_SEARCH'),
            message: _('Search path \'{0}\' is already handled by another entry', tp)
          });
        }

        if ( !paths.length || !c ) {
          paths.push(tp);
        }

      });

      var searchSettings = {
        applications: win._find('SearchEnableApplications').get('value'),
        files: win._find('SearchEnableFiles').get('value'),
        paths: paths
      };

      return function(cb) {
        var sm = OSjs.Core.getSettingsManager();
        sm.instance('SearchEngine').set(null, searchSettings, cb, false);
      };
    }
  };

  /////////////////////////////////////////////////////////////////////////////
  // EXPORTS
  /////////////////////////////////////////////////////////////////////////////

  OSjs.Applications = OSjs.Applications || {};
  OSjs.Applications.ApplicationSettings = OSjs.Applications.ApplicationSettings || {};
  OSjs.Applications.ApplicationSettings.Modules = OSjs.Applications.ApplicationSettings.Modules || {};
  OSjs.Applications.ApplicationSettings.Modules.Search = module;

})(OSjs.Core.Application, OSjs.Core.Window, OSjs.Utils, OSjs.API, OSjs.VFS, OSjs.GUI);

/*!
 * OS.js - JavaScript Cloud/Web Sound Platform
 *
 * Copyright (c) 2011-2016, Anders Evenrud <andersevenrud@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @author  Anders Evenrud <andersevenrud@gmail.com>
 * @licence Simplified BSD License
 */
(function(Application, Window, Utils, API, VFS, GUI) {
  'use strict';

  var sounds = {};

  function renderList(win, scheme) {
    win._find('SoundsList').clear().add(Object.keys(sounds).map(function(name) {
      return {
        value: {
          name: name,
          value: sounds[name]
        },
        columns: [
          {label: name},
          {label: sounds[name]}
        ]
      };
    }));
  }

  function editList(win, scheme, key) {
    var _ = OSjs.Applications.ApplicationSettings._;
    win._toggleDisabled(true);
    API.createDialog('Input', {
      message: _('Enter filename for:') + ' ' + key.name,
      value: key.value
    }, function(ev, button, value) {
      win._toggleDisabled(false);
      value = value || '';
      if ( value.length ) {
        sounds[key.name] = value;
      }

      renderList(win, scheme);
    })
  }

  /////////////////////////////////////////////////////////////////////////////
  // MODULE
  /////////////////////////////////////////////////////////////////////////////

  var module = {
    group: 'personal',
    name: 'Sounds',
    label: 'LBL_SOUNDS',
    icon: 'status/stock_volume-max.png',

    init: function() {
    },

    update: function(win, scheme, settings, wm) {
      win._find('SoundThemeName').set('value', settings.soundTheme);
      win._find('EnableSounds').set('value', settings.enableSounds);

      sounds = Utils.cloneObject(settings.sounds);

      renderList(win, scheme);
    },

    render: function(win, scheme, root, settings, wm) {
      var soundThemes = (function(tmp) {
        return Object.keys(tmp).map(function(t) {
          return {label: tmp[t], value: t};
        });
      })(wm.getSoundThemes());

      win._find('SoundThemeName').add(soundThemes);

      win._find('SoundsEdit').on('click', function() {
        var selected = win._find('SoundsList').get('selected');
        if ( selected && selected[0] ) {
          editList(win, scheme, selected[0].data);
        }
      });
    },

    save: function(win, scheme, settings, wm) {
      settings.soundTheme = win._find('SoundThemeName').get('value');
      settings.enableSounds = win._find('EnableSounds').get('value');

      if ( sounds && Object.keys(sounds).length ) {
        settings.sounds = sounds;
      }
    }
  };

  /////////////////////////////////////////////////////////////////////////////
  // EXPORTS
  /////////////////////////////////////////////////////////////////////////////

  OSjs.Applications = OSjs.Applications || {};
  OSjs.Applications.ApplicationSettings = OSjs.Applications.ApplicationSettings || {};
  OSjs.Applications.ApplicationSettings.Modules = OSjs.Applications.ApplicationSettings.Modules || {};
  OSjs.Applications.ApplicationSettings.Modules.Sounds = module;

})(OSjs.Core.Application, OSjs.Core.Window, OSjs.Utils, OSjs.API, OSjs.VFS, OSjs.GUI);

/*!
 * OS.js - JavaScript Cloud/Web Locale Platform
 *
 * Copyright (c) 2011-2016, Anders Evenrud <andersevenrud@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @author  Anders Evenrud <andersevenrud@gmail.com>
 * @licence Simplified BSD License
 */
(function(Application, Window, Utils, API, VFS, GUI) {
  'use strict';

  /////////////////////////////////////////////////////////////////////////////
  // MODULE
  /////////////////////////////////////////////////////////////////////////////

  var module = {
    group: 'user',
    name: 'Locale',
    label: 'LBL_LOCALE',
    icon: 'apps/locale.png',

    init: function() {
    },

    update: function(win, scheme, settings, wm) {
      var config = OSjs.Core.getConfig();
      var locales = config.Languages;

      win._find('UserLocale').clear().add(Object.keys(locales).filter(function(l) {
        return !!OSjs.Locales[l];
      }).map(function(l) {
        return {label: locales[l], value: l};
      })).set('value', API.getLocale());
    },

    render: function(win, scheme, root, settings, wm) {
    },

    save: function(win, scheme, settings, wm) {
      settings.language = win._find('UserLocale').get('value');
    }
  };

  /////////////////////////////////////////////////////////////////////////////
  // EXPORTS
  /////////////////////////////////////////////////////////////////////////////

  OSjs.Applications = OSjs.Applications || {};
  OSjs.Applications.ApplicationSettings = OSjs.Applications.ApplicationSettings || {};
  OSjs.Applications.ApplicationSettings.Modules = OSjs.Applications.ApplicationSettings.Modules || {};
  OSjs.Applications.ApplicationSettings.Modules.Locale = module;

})(OSjs.Core.Application, OSjs.Core.Window, OSjs.Utils, OSjs.API, OSjs.VFS, OSjs.GUI);

/*!
 * OS.js - JavaScript Cloud/Web Desktop Platform
 *
 * Copyright (c) 2011-2016, Anders Evenrud <andersevenrud@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @author  Anders Evenrud <andersevenrud@gmail.com>
 * @licence Simplified BSD License
 */
(function(Application, Window, Utils, API, VFS, GUI) {
  'use strict';

  var hotkeys = {};

  function renderList(win, scheme) {
    win._find('HotkeysList').clear().add(Object.keys(hotkeys).map(function(name) {
      return {
        value: {
          name: name,
          value: hotkeys[name]
        },
        columns: [
          {label: name},
          {label: hotkeys[name]}
        ]
      };
    }));
  }

  function editList(win, scheme, key) {
    var _ = OSjs.Applications.ApplicationSettings._;

    win._toggleDisabled(true);
    API.createDialog('Input', {
      message: _('Enter shortcut for:') + ' ' + key.name,
      value: key.value
    }, function(ev, button, value) {
      win._toggleDisabled(false);
      value = value || '';
      if ( value.indexOf('+') !== -1 ) {
        hotkeys[key.name] = value;
      }

      renderList(win, scheme);
    })
  }

  /////////////////////////////////////////////////////////////////////////////
  // MODULE
  /////////////////////////////////////////////////////////////////////////////

  var module = {
    group: 'personal',
    name: 'Input',
    label: 'LBL_INPUT',
    icon: 'apps/key_bindings.png',

    init: function() {
    },

    update: function(win, scheme, settings, wm) {
      win._find('EnableHotkeys').set('value', settings.enableHotkeys);

      hotkeys = Utils.cloneObject(settings.hotkeys);

      renderList(win, scheme);
    },

    render: function(win, scheme, root, settings, wm) {
      win._find('HotkeysEdit').on('click', function() {
        var selected = win._find('HotkeysList').get('selected');
        if ( selected && selected[0] ) {
          editList(win, scheme, selected[0].data);
        }
      });
    },

    save: function(win, scheme, settings, wm) {
      settings.enableHotkeys = win._find('EnableHotkeys').get('value');
      if ( hotkeys && Object.keys(hotkeys).length ) {
        settings.hotkeys = hotkeys;
      }
    }
  };

  /////////////////////////////////////////////////////////////////////////////
  // EXPORTS
  /////////////////////////////////////////////////////////////////////////////

  OSjs.Applications = OSjs.Applications || {};
  OSjs.Applications.ApplicationSettings = OSjs.Applications.ApplicationSettings || {};
  OSjs.Applications.ApplicationSettings.Modules = OSjs.Applications.ApplicationSettings.Modules || {};
  OSjs.Applications.ApplicationSettings.Modules.Input = module;

})(OSjs.Core.Application, OSjs.Core.Window, OSjs.Utils, OSjs.API, OSjs.VFS, OSjs.GUI);

/*!
 * OS.js - JavaScript Cloud/Web VFS Platform
 *
 * Copyright (c) 2011-2016, Anders Evenrud <andersevenrud@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @author  Anders Evenrud <andersevenrud@gmail.com>
 * @licence Simplified BSD License
 */
(function(Application, Window, Utils, API, VFS, GUI) {
  'use strict';

  function createMountWindow(win, scheme, selected, ondone) {

    var nwin = new Window('SettingsMountWindow', {
      icon: win._app.__metadata.icon,
      title: win._app.__metadata.name,
      width: 400,
      height: 440
    }, win._app, scheme);

    nwin._on('destroy', function(root) {
      win._toggleDisabled(false);
    });

    nwin._on('inited', function(root) {
      win._toggleDisabled(true);
    });

    nwin._on('init', function(root) {
      var self = this;

      function add(conn) {
        try {
          OSjs.Core.getMountManager().add(conn);
        } catch ( e ) {
          API.error(self._title, 'An error occured while trying to mount', e);
          console.warn(e.stack, e);
          return false;
        }
        return true;
      }

      function done() {
        var conn = {
          transport: scheme.find(self, 'MountType').get('value'),
          name: scheme.find(self, 'MountName').get('value'),
          description: scheme.find(self, 'MountDescription').get('value'),
          options: {
            host: scheme.find(self, 'MountHost').get('value'),
            ns: scheme.find(self, 'MountNamespace').get('value'),
            username: scheme.find(self, 'MountUsername').get('value'),
            password: scheme.find(self, 'MountPassword').get('value'),
            cors: scheme.find(self, 'MountCORS').get('value')
          }
        };

        if ( selected ) {
          try {
            OSjs.Core.getMountManager().remove(selected.name, function() {
              if ( add(conn) ) {
                ondone(conn, selected);
              }
              self._close();
            });
            return;
          } catch ( e ) {
            console.warn('Settings Mount modification failure', e, e.stack);
          }
        } else {
          if ( !add(conn) ) {
            conn = null;
          }
        }

        self._close();
        ondone();
      }

      scheme.render(this, this._name, root)

      if ( selected ) {
        scheme.find(self, 'MountType').set('value', selected.transport);
        scheme.find(self, 'MountName').set('value', selected.name);
        scheme.find(self, 'MountDescription').set('value', selected.description);
        if ( selected.options ) {
          scheme.find(self, 'MountHost').set('value', selected.options.host);
          scheme.find(self, 'MountNamespace').set('value', selected.options.ns);
          scheme.find(self, 'MountUsername').set('value', selected.options.username);
          scheme.find(self, 'MountPassword').set('value', selected.options.password);
          scheme.find(self, 'MountCORS').set('value', selected.options.cors);
        }
      }

      scheme.find(this, 'ButtonClose').on('click', function() {
        self._close();
      });

      scheme.find(this, 'ButtonOK').on('click', function() {
        done();
      });
    });

    return win._addChild(nwin, true, true);
  }

  function renderMounts(win, scheme) {
    var sm = OSjs.Core.getSettingsManager();
    var sf = sm.instance('VFS');
    var entries = sf.get('mounts', []).map(function(i, idx) {
      return {
        value: idx,
        columns: [
          {label: i.name},
          {label: i.description}
        ]
      };
    });

    win._find('MountList').clear().add(entries);
  }

  function _save(sf, win, scheme, mounts) {
    win._toggleLoading(true);
    sf.set(null, {mounts: mounts}, function() {
      renderMounts(win, scheme);
      win._toggleLoading(false);
    }, false);
  }

  function removeMount(win, scheme, index) {
    var sm = OSjs.Core.getSettingsManager();
    var sf = sm.instance('VFS');
    var mounts = sf.get('mounts', []);

    if ( typeof mounts[index] !== 'undefined' ) {
      mounts.splice(index, 1);
      _save(sf, win, scheme, mounts);
    }
  }

  function addMount(conn, replace, win, scheme) {
    if ( !conn ) {
      return;
    }

    var sm = OSjs.Core.getSettingsManager();
    var sf = sm.instance('VFS');
    var mounts = sf.get('mounts', []).filter(function(iter) {
      if ( replace && replace.name === iter.name ) {
        return false;
      }
      return true;
    });
    mounts.push(conn);

    _save(sf, win, scheme, mounts);
  }

  /////////////////////////////////////////////////////////////////////////////
  // MODULE
  /////////////////////////////////////////////////////////////////////////////

  var module = {
    group: 'system',
    name: 'VFS',
    label: 'VFS',
    icon: 'devices/harddrive.png',
    watch: ['VFS'],

    init: function(app) {
    },

    update: function(win, scheme, settings, wm) {
      var vfsOptions = Utils.cloneObject(OSjs.Core.getSettingsManager().get('VFS') || {});
      var scandirOptions = vfsOptions.scandir || {};

      win._find('ShowFileExtensions').set('value', scandirOptions.showFileExtensions === true);
      win._find('ShowHiddenFiles').set('value', scandirOptions.showHiddenFiles === true);

      renderMounts(win, scheme);
    },

    render: function(win, scheme, root, settings, wm) {
      function ondone(connection, replace) {
        addMount(connection, replace, win, scheme);
      }

      win._find('MountList').set('columns', [
        {label: 'Name'},
        {label: 'Description'}
      ]);

      win._find('MountRemove').on('click', function() {
        var sel = win._find('MountList').get('selected');
        if ( sel && sel.length ) {
          removeMount(win, scheme, sel[0].data);
        }
      });

      win._find('MountAdd').on('click', function() {
        createMountWindow(win, scheme, null, ondone);
      });

      win._find('MountEdit').on('click', function() {
        var sel = win._find('MountList').get('selected');
        var sm = OSjs.Core.getSettingsManager();
        var mounts = sm.instance('VFS').get('mounts', []);
        if ( sel && sel.length ) {
          var mount = mounts[sel[0].data];
          if ( mount ) {
            createMountWindow(win, scheme, mount, ondone);
          }
        }
      });
    },

    save: function(win, scheme, settings, wm) {
      var vfsSettings = {
        scandir: {
          showHiddenFiles: win._find('ShowHiddenFiles').get('value'),
          showFileExtensions: win._find('ShowFileExtensions').get('value')
        }
      };

      return function(cb) {
        var sm = OSjs.Core.getSettingsManager();
        sm.instance('VFS').set(null, vfsSettings, cb, false);
      };
    }
  };

  /////////////////////////////////////////////////////////////////////////////
  // EXPORTS
  /////////////////////////////////////////////////////////////////////////////

  OSjs.Applications = OSjs.Applications || {};
  OSjs.Applications.ApplicationSettings = OSjs.Applications.ApplicationSettings || {};
  OSjs.Applications.ApplicationSettings.Modules = OSjs.Applications.ApplicationSettings.Modules || {};
  OSjs.Applications.ApplicationSettings.Modules.VFS = module;

})(OSjs.Core.Application, OSjs.Core.Window, OSjs.Utils, OSjs.API, OSjs.VFS, OSjs.GUI);

/*!
 * OS.js - JavaScript Cloud/Web PM Platform
 *
 * Copyright (c) 2011-2016, Anders Evenrud <andersevenrud@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @author  Anders Evenrud <andersevenrud@gmail.com>
 * @licence Simplified BSD License
 */
(function(Application, Window, Utils, API, PM, GUI, VFS) {
  'use strict';

  var list, hidden;

  function updateEnabledStates() {
    var pacman = OSjs.Core.getPackageManager();
    var sm = OSjs.Core.getSettingsManager();
    var pool = sm.instance('PackageManager', {Hidden: []});

    list = pacman.getPackages(false);
    hidden = pool.get('Hidden');
  }

  function renderInstalled(win, scheme) {
    win._find('ButtonUninstall').set('disabled', true);

    updateEnabledStates();

    var view = win._find('InstalledPackages');
    var rows = [];

    Object.keys(list).forEach(function(k, idx) {
      rows.push({
        index: idx,
        value: k,
        columns: [
          {label: ''},
          {label: k},
          {label: list[k].scope},
          {label: list[k].name}
        ]
      });
    });

    view.clear();
    view.add(rows);

    view.$element.querySelectorAll('gui-list-view-body > gui-list-view-row').forEach(function(row) {
      var col = row.children[0];
      var name = row.getAttribute('data-value');
      var enabled = hidden.indexOf(name) >= 0;

      scheme.create(win, 'gui-checkbox', {value: enabled}, col).on('change', function(ev) {
        var idx = hidden.indexOf(name);

        if ( ev.detail ) {
          if ( idx < 0 ) {
            hidden.push(name);
          }
        } else {
          if ( idx >= 0 ) {
            hidden.splice(idx, 1);
          }
        }
      });
    });
  }

  function renderPaths(win, scheme) {
    var sm = OSjs.Core.getSettingsManager();
    var paths = sm.instance('PackageManager').get('PackagePaths', []);
    win._find('PackagePaths').clear().add(paths.map(function(iter, idx) {
      return {
        value: idx,
        columns: [
          {label: iter}
        ]
      };
    }));
  }

  function _save(sf, win, scheme, paths) {
    win._toggleLoading(true);
    sf.set(null, {PackagePaths: paths}, function() {
      renderPaths(win, scheme);
      win._toggleLoading(false);
    }, false);
  }

  function addPath(win, scheme) {
    var sm = OSjs.Core.getSettingsManager();
    var sf = sm.instance('PackageManager');
    var paths = sf.get('PackagePaths', []);

    win._toggleDisabled(true);
    API.createDialog('Input', {
      message: 'Enter path',
      placeholder: 'mount:///path'
    }, function(ev, btn, value) {
      win._toggleDisabled(false);

      if ( value ) {
        if ( paths.indexOf(value) === -1 ) {
          paths.push(value);
          _save(sf, win, scheme, paths);
        }
      }
    });
  }

  function removePath(win, scheme, index) {
    var sm = OSjs.Core.getSettingsManager();
    var sf = sm.instance('PackageManager');
    var paths = sf.get('PackagePaths', []);
    if ( typeof paths[index] !== 'undefined' ) {
      paths.splice(index, 1);
      _save(sf, win, scheme, paths);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // MODULE
  /////////////////////////////////////////////////////////////////////////////

  var module = {
    group: 'misc',
    name: 'Packages',
    label: 'LBL_PACKAGES',
    icon: 'apps/system-software-install.png',
    button: false,

    init: function() {
    },

    update: function(win, scheme, settings, wm) {
      renderInstalled(win, scheme);
      renderPaths(win, scheme);
    },

    render: function(win, scheme, root, settings, wm) {
      var pacman = OSjs.Core.getPackageManager();
      var sm = OSjs.Core.getSettingsManager();
      var pool = sm.instance('PackageManager', {Hidden: []});

      win._find('ButtonUninstall').on('click', function() {
        var selected = win._find('InstalledPackages').get('selected');
        if ( selected && selected[0] ) {
          var pkg = pacman.getPackage(selected[0].data);
          if ( pkg && pkg.scope === 'user' ) {
            win._toggleLoading(true);

            var file = new VFS.File(pkg.path);
            pacman.uninstall(file, function(e) {
              win._toggleLoading(false);
              renderInstalled(win, scheme);

              if ( e ) {
                alert(e);
              }
            });
          }
        }
      });

      win._find('InstalledPackages').on('select', function(ev) {
        var d = true;
        var e = ev.detail.entries || [];
        if ( e.length ) {
          var pkg = pacman.getPackage(e[0].data);
          if ( pkg && pkg.scope === 'user' ) {
            d = false;
          }
        }

        win._find('ButtonUninstall').set('disabled', d);
      });

      win._find('ButtonSaveHidden').on('click', function() {
        win._toggleLoading(true);
        pool.set('Hidden', hidden, function() {
          win._toggleLoading(false);
        });
      });

      win._find('ButtonRegen').on('click', function() {
        win._toggleLoading(true);
        pacman.generateUserMetadata(function() {
          win._toggleLoading(false);

          renderInstalled(win, scheme);
        });
      });

      win._find('ButtonZipInstall').on('click', function() {
        win._toggleDisabled(true);

        API.createDialog('File', {
          filter: ['application/zip']
        }, function(ev, button, result) {
          if ( button !== 'ok' || !result ) {
            win._toggleDisabled(false);
          } else {
            OSjs.Core.getPackageManager().install(result, true, function(e) {
              win._toggleDisabled(false);
              renderInstalled(win, scheme);

              if ( e ) {
                alert(e);
              }
            });
          }
        }, win);
      });

      win._find('PackagePathsRemove').on('click', function() {
        var sel = win._find('PackagePaths').get('selected');
        if ( sel && sel.length ) {
          removePath(win, scheme, sel[0].data);
        }
      });

      win._find('PackagePathsAdd').on('click', function() {
        addPath(win, scheme);
      });
    },

    save: function(win, scheme, settings, wm) {
    }
  };

  /////////////////////////////////////////////////////////////////////////////
  // EXPORTS
  /////////////////////////////////////////////////////////////////////////////

  OSjs.Applications = OSjs.Applications || {};
  OSjs.Applications.ApplicationSettings = OSjs.Applications.ApplicationSettings || {};
  OSjs.Applications.ApplicationSettings.Modules = OSjs.Applications.ApplicationSettings.Modules || {};
  OSjs.Applications.ApplicationSettings.Modules.PM = module;

})(OSjs.Core.Application, OSjs.Core.Window, OSjs.Utils, OSjs.API, OSjs.PM, OSjs.GUI, OSjs.VFS);

/*!
 * OS.js - JavaScript Cloud/Web PM Platform
 *
 * Copyright (c) 2011-2016, Anders Evenrud <andersevenrud@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @author  Anders Evenrud <andersevenrud@gmail.com>
 * @licence Simplified BSD License
 */
(function(Application, Window, Utils, API, PM, GUI, VFS) {
  'use strict';

  function installSelected(download, cb) {
    var pacman = OSjs.Core.getPackageManager();

    var file = new VFS.File(download, 'application/zip');
    VFS.read(file, function(error, ab) {
      if ( error ) {
        cb(error);
        return;
      }

      var dest = new VFS.File({
        filename: Utils.filename(download),
        type: 'file',
        path: 'home:///' + Utils.filename(download),
        mime: 'application/zip'
      });

      VFS.write(dest, ab, function(error, success) {
        if ( error ) {
          cb('Failed to write package: ' + error); // FIXME
          return;
        }

        OSjs.Core.getPackageManager().install(dest, true, function(error) {
          if ( error ) {
            cb('Failed to install package: ' + error); // FIXME
            return;
          }
          pacman.generateUserMetadata(function() {
            cb(false, true);
          });
        });
      });
    });
  }

  function renderStore(win) {
    win._toggleLoading(true);

    var pacman = OSjs.Core.getPackageManager();
    pacman.getStorePackages({}, function(error, result) {
      var rows = result.map(function(i, idx) {
        var a = document.createElement('a');
        a.href = i._repository;

        return {
          index: idx,
          value: i.download,
          columns: [
            {label: i.name},
            {label: a.hostname},
            {label: i.version},
            {label: i.author}
          ]
        };
      });

      win._toggleLoading(false);

      win._find('AppStorePackages').clear().add(rows);
    });
  }

  /////////////////////////////////////////////////////////////////////////////
  // MODULE
  /////////////////////////////////////////////////////////////////////////////

  var module = {
    group: 'user',
    name: 'Store',
    label: 'LBL_STORE',
    icon: 'apps/system-software-update.png',
    button: false,

    init: function() {
    },

    update: function(win, scheme, settings, wm, clicked) {
      if ( clicked ) {
        renderStore(win);
      }
    },

    render: function(win, scheme, root, settings, wm) {
      win._find('ButtonStoreRefresh').on('click', function() {
        renderStore(win);
      });

      win._find('ButtonStoreInstall').on('click', function() {
        var selected = win._find('AppStorePackages').get('selected');
        if ( selected.length && selected[0].data ) {
          win._toggleLoading(true);
          installSelected(selected[0].data, function(error, result) {
            win._toggleLoading(false);
            if ( error ) {
              alert(error); // FIXME
              return;
            }
          });
        }
      });
    },

    save: function(win, scheme, settings, wm) {
    }
  };

  /////////////////////////////////////////////////////////////////////////////
  // EXPORTS
  /////////////////////////////////////////////////////////////////////////////

  OSjs.Applications = OSjs.Applications || {};
  OSjs.Applications.ApplicationSettings = OSjs.Applications.ApplicationSettings || {};
  OSjs.Applications.ApplicationSettings.Modules = OSjs.Applications.ApplicationSettings.Modules || {};
  OSjs.Applications.ApplicationSettings.Modules.Store = module;

})(OSjs.Core.Application, OSjs.Core.Window, OSjs.Utils, OSjs.API, OSjs.PM, OSjs.GUI, OSjs.VFS);

/*!
 * OS.js - JavaScript Cloud/Web Panel Platform
 *
 * Copyright (c) 2011-2016, Anders Evenrud <andersevenrud@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @author  Anders Evenrud <andersevenrud@gmail.com>
 * @licence Simplified BSD License
 */
(function(Application, Window, Utils, API, Panel, GUI) {
  'use strict';

  var panelItems = [];
  var items = [];
  var max = 0;
  var panel;

  /////////////////////////////////////////////////////////////////////////////
  // WINDOWS
  /////////////////////////////////////////////////////////////////////////////

  function PanelItemDialog(app, metadata, scheme, callback) {
    Window.apply(this, ['ApplicationSettingsPanelItemsWindow', {
      icon: metadata.icon,
      title: metadata.name + ' - Panel Items',
      width: 400,
      height: 300
    }, app, scheme]);

    this.callback = callback;
    this.closed = false;
  }

  PanelItemDialog.prototype = Object.create(Window.prototype);
  PanelItemDialog.constructor = Window;

  PanelItemDialog.prototype.init = function(wm, app, scheme) {
    var self = this;
    var root = Window.prototype.init.apply(this, arguments);

    // Load and set up scheme (GUI) here
    scheme.render(this, 'PanelSettingWindow', root, null, null, {
      _: OSjs.Applications.ApplicationSettings._
    });

    var pacman = OSjs.Core.getPackageManager();
    var avail = pacman.getPackage('CoreWM').panelItems;
    scheme.find(this, 'List').clear().add(Object.keys(avail).map(function(i, idx) {
      return {
        value: i,
        columns: [{
          icon: API.getIcon(avail[i].Icon),
          label: Utils.format('{0} ({1})', avail[i].Name, avail[i].Description)
        }]
      };
    }));

    scheme.find(this, 'ButtonPanelOK').on('click', function() {
      self.closed = true;
      var selected = scheme.find(self, 'List').get('selected');
      self.callback('ok', selected.length ? selected[0] : null);
      self._close();
    });

    scheme.find(this, 'ButtonPanelCancel').on('click', function() {
      self._close();
    });

    return root;
  };

  PanelItemDialog.prototype._close = function() {
    if ( !this.closed ) {
      this.callback('cancel');
    }
    return Window.prototype._close.apply(this, arguments);
  };

  /////////////////////////////////////////////////////////////////////////////
  // HELPERS
  /////////////////////////////////////////////////////////////////////////////

  function openOptions(wm, idx) {
    // FIXME
    try {
      wm.panels[0]._items[idx].openSettings();
    } catch ( e ) {}
  }

  function checkSelection(win, idx) {
    var hasOptions = true;

    try {
      var it = items[panel.items[idx].name];
      hasOptions = it.HasOptions === true;
    } catch ( e ) {}

    win._find('PanelButtonOptions').set('disabled', idx < 0 || !hasOptions);
    win._find('PanelButtonRemove').set('disabled', idx < 0);
    win._find('PanelButtonUp').set('disabled', idx <= 0);
    win._find('PanelButtonDown').set('disabled', idx < 0 || idx >= max);
  }

  function renderItems(win, setSelected) {
    var list = [];

    panelItems.forEach(function(i, idx) {
      var name = i.name;

      if ( items[name] ) {
        list.push({
          value: idx,
          columns: [{
            icon: API.getIcon(items[name].Icon),
            label: Utils.format('{0} ({1})', items[name].Name, items[name].Description)
          }]
        });
      }
    });
    max = panelItems.length - 1;

    var view = win._find('PanelItems');
    view.clear();
    view.add(list);

    if ( typeof setSelected !== 'undefined' ) {
      view.set('selected', setSelected);
      checkSelection(win, setSelected);
    } else {
      checkSelection(win, -1);
    }
  }

  function movePanelItem(win, index, pos) {
    var value = panelItems[index];
    var newIndex = index + pos;
    panelItems.splice(index, 1);
    panelItems.splice(newIndex, 0, value);
    renderItems(win, newIndex);
  }

  function createDialog(win, scheme, cb) {
    if ( scheme ) {
      var app = win._app;
      win._addChild(new PanelItemDialog(app, app.__metadata, scheme, cb), true, true);
    }
  }

  function createColorDialog(win, color, cb) {
    win._toggleDisabled(true);

    API.createDialog('Color', {
      color: color
    }, function(ev, button, result) {
      win._toggleDisabled(false);
      if ( button === 'ok' && result ) {
        cb(result.hex);
      }
    }, win);
  }

  /////////////////////////////////////////////////////////////////////////////
  // MODULE
  /////////////////////////////////////////////////////////////////////////////

  var module = {
    group: 'personal',
    name: 'Panel',
    label: 'LBL_PANELS',
    icon: 'apps/gnome-panel.png',

    init: function() {
    },

    update: function(win, scheme, settings, wm) {
      panel = settings.panels[0];

      var opacity = 85;
      if ( typeof panel.options.opacity === 'number' ) {
        opacity = panel.options.opacity;
      }

      win._find('PanelPosition').set('value', panel.options.position);
      win._find('PanelAutoHide').set('value', panel.options.autohide);
      win._find('PanelOntop').set('value', panel.options.ontop);
      win._find('PanelBackgroundColor').set('value', panel.options.background || '#101010');
      win._find('PanelForegroundColor').set('value', panel.options.foreground || '#ffffff');
      win._find('PanelOpacity').set('value', opacity);

      items = OSjs.Core.getPackageManager().getPackage('CoreWM').panelItems;

      panelItems = panel.items || [];

      renderItems(win);
    },

    render: function(win, scheme, root, settings, wm) {
      win._find('PanelPosition').add([
        {value: 'top',    label: API._('LBL_TOP')},
        {value: 'bottom', label: API._('LBL_BOTTOM')}
      ]);

      win._find('PanelBackgroundColor').on('open', function(ev) {
        createColorDialog(win, ev.detail, function(result) {
          win._find('PanelBackgroundColor').set('value', result);
        });
      });

      win._find('PanelForegroundColor').on('open', function(ev) {
        createColorDialog(win, ev.detail, function(result) {
          win._find('PanelForegroundColor').set('value', result);
        });
      });

      win._find('PanelItems').on('select', function(ev) {
        if ( ev && ev.detail && ev.detail.entries && ev.detail.entries.length ) {
          checkSelection(win, ev.detail.entries[0].index);
        }
      });

      win._find('PanelButtonAdd').on('click', function() {
        win._toggleDisabled(true);
        createDialog(win, scheme, function(ev, result) {
          win._toggleDisabled(false);

          if ( result ) {
            panelItems.push({name: result.data});
            renderItems(win);
          }
        });
      });

      win._find('PanelButtonRemove').on('click', function() {
        var selected = win._find('PanelItems').get('selected');
        if ( selected.length ) {
          panelItems.splice(selected[0].index, 1);
          renderItems(win);
        }
      });

      win._find('PanelButtonUp').on('click', function() {
        var selected = win._find('PanelItems').get('selected');
        if ( selected.length ) {
          movePanelItem(win, selected[0].index, -1);
        }
      });
      win._find('PanelButtonDown').on('click', function() {
        var selected = win._find('PanelItems').get('selected');
        if ( selected.length ) {
          movePanelItem(win, selected[0].index, 1);
        }
      });

      win._find('PanelButtonReset').on('click', function() {
        var defaults = wm.getDefaultSetting('panels');
        panelItems = defaults[0].items;
        renderItems(win);
      });

      win._find('PanelButtonOptions').on('click', function() {
        var selected = win._find('PanelItems').get('selected');
        if ( selected.length ) {
          openOptions(wm, selected[0].index);
        }
      });
    },

    save: function(win, scheme, settings, wm) {
      settings.panels = settings.panels || [{}];
      settings.panels[0].options = settings.panels[0].options || {};

      settings.panels[0].options.position = win._find('PanelPosition').get('value');
      settings.panels[0].options.autohide = win._find('PanelAutoHide').get('value');
      settings.panels[0].options.ontop = win._find('PanelOntop').get('value');
      settings.panels[0].options.background = win._find('PanelBackgroundColor').get('value') || '#101010';
      settings.panels[0].options.foreground = win._find('PanelForegroundColor').get('value') || '#ffffff';
      settings.panels[0].options.opacity = win._find('PanelOpacity').get('value');
      settings.panels[0].items = panelItems;
    }
  };

  /////////////////////////////////////////////////////////////////////////////
  // EXPORTS
  /////////////////////////////////////////////////////////////////////////////

  OSjs.Applications = OSjs.Applications || {};
  OSjs.Applications.ApplicationSettings = OSjs.Applications.ApplicationSettings || {};
  OSjs.Applications.ApplicationSettings.Modules = OSjs.Applications.ApplicationSettings.Modules || {};
  OSjs.Applications.ApplicationSettings.Modules.Panel = module;

})(OSjs.Core.Application, OSjs.Core.Window, OSjs.Utils, OSjs.API, OSjs.Panel, OSjs.GUI);

/*!
 * OS.js - JavaScript Cloud/Web User Platform
 *
 * Copyright (c) 2011-2016, Anders Evenrud <andersevenrud@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @author  Anders Evenrud <andersevenrud@gmail.com>
 * @licence Simplified BSD License
 */
(function(Application, Window, Utils, API, User, GUI) {
  'use strict';

  /////////////////////////////////////////////////////////////////////////////
  // MODULE
  /////////////////////////////////////////////////////////////////////////////

  var module = {
    group: 'user',
    name: 'User',
    label: 'LBL_USER',
    icon: 'apps/user-info.png',
    button: false,

    init: function() {
    },

    update: function(win, scheme, settings, wm) {
      var user = OSjs.Core.getHandler().getUserData();

      win._find('UserID').set('value', user.id);
      win._find('UserName').set('value', user.name);
      win._find('UserUsername').set('value', user.username);
      win._find('UserGroups').set('value', user.groups);
    },

    render: function(win, scheme, root, settings, wm) {
    },

    save: function(win, scheme, settings, wm) {
    }
  };

  /////////////////////////////////////////////////////////////////////////////
  // EXPORTS
  /////////////////////////////////////////////////////////////////////////////

  OSjs.Applications = OSjs.Applications || {};
  OSjs.Applications.ApplicationSettings = OSjs.Applications.ApplicationSettings || {};
  OSjs.Applications.ApplicationSettings.Modules = OSjs.Applications.ApplicationSettings.Modules || {};
  OSjs.Applications.ApplicationSettings.Modules.User = module;

})(OSjs.Core.Application, OSjs.Core.Window, OSjs.Utils, OSjs.API, OSjs.User, OSjs.GUI);

/*!
 * OS.js - JavaScript Cloud/Web User Platform
 *
 * Copyright (c) 2011-2016, Anders Evenrud <andersevenrud@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @author  Anders Evenrud <andersevenrud@gmail.com>
 * @licence Simplified BSD License
 */
(function(Application, Window, Utils, API, User, GUI) {
  'use strict';

  function renderUsers(win, scheme) {
    API.call('users', {command: 'list'}, function(err, users) {
      if ( users instanceof Array ) {
        win._find('UsersList').clear().add(users.map(function(iter, idx) {
          return {
            value: iter,
            columns: [
              {label: iter.username},
              {label: iter.name}
            ]
          };
        }));
      }
    });
  }

  function showDialog(win, scheme, data, passwd) {
    var _ = OSjs.Applications.ApplicationSettings._;

    data = data || {};
    win._toggleDisabled(true);

    if ( passwd ) {
      API.createDialog('Input', {
        message: _('Set user password'),
        type: 'password'
      }, function(ev, button, value) {
        if ( !value ) {
          win._toggleDisabled(false);
          return;
        }

        API.call('users', {command: 'passwd', user: {password: value}}, function(err, users) {
          win._toggleDisabled(false);
          if ( err ) {
            API.error('Settings', _('Error while managing users'), err);
          }
          renderUsers(win, scheme);
        });
      });
      return;
    }

    var nwin = new Window('SettingsUserWindow', {
      icon: win._app.__metadata.icon,
      title: win._app.__metadata.name,
      width: 400,
      height: 250
    }, win._app, scheme);

    nwin._on('destroy', function(root) {
      win._toggleDisabled(false);
    });

    nwin._on('init', function(root) {
      var self = this;

      scheme.render(this, this._name, root)

      if ( Object.keys(data).length ) {
        scheme.find(self, 'UserUsername').set('value', data.username);
        scheme.find(self, 'UserName').set('value', data.name);
        scheme.find(self, 'UserGroups').set('value', JSON.stringify(data.groups));
      }

      scheme.find(this, 'ButtonClose').on('click', function() {
        self._close();
      });

      scheme.find(this, 'ButtonOK').on('click', function() {
        data.username = scheme.find(self, 'UserUsername').get('value');
        data.name = scheme.find(self, 'UserName').get('value') || data.username;
        data.groups = [];

        try {
          data.groups = JSON.parse(scheme.find(self, 'UserGroups').get('value'));
        } catch ( e ) {
        }

        if ( !data.username || !data.groups ) {
          return self._close();
        }

        API.call('users', {command: 'edit', user: data}, function(err, users) {
          if ( err ) {
            API.error('Settings', _('Error while managing users'), err);
          }
          renderUsers(win, scheme);

          self._close();
        });
      });
    });

    return win._addChild(nwin, true, true);
  }

  function removeUser(win, scheme, data) {
    var _ = OSjs.Applications.ApplicationSettings._;

    API.call('users', {command: 'remove', user: {id: data.id}}, function(err, users) {
      if ( err ) {
        API.error('Settings', _('Error while managing users'), err);
      }
      renderUsers(win, scheme);
    });
  }

  /////////////////////////////////////////////////////////////////////////////
  // MODULE
  /////////////////////////////////////////////////////////////////////////////

  var module = {
    group: 'system',
    name: 'Users',
    label: 'LBL_USERS',
    icon: 'apps/system-users.png',
    button: false,

    init: function() {
    },

    update: function(win, scheme, settings, wm) {
      renderUsers(win, scheme);
    },

    render: function(win, scheme, root, settings, wm) {
      function _action(cb, te) {
        var sel = win._find('UsersList').get('selected');
        if ( sel && sel.length ) {
          cb(sel[0].data)
        } else {
          if ( te ) {
            cb(null);
          }
        }
      }
      win._find('UsersAdd').on('click', function() {
        _action(function(data) {
          showDialog(win, scheme, data)
        }, true);
      });
      win._find('UsersRemove').on('click', function() {
        _action(function(data) {
          removeUser(win, scheme, data);
        });
      });
      win._find('UsersEdit').on('click', function() {
        _action(function(data) {
          showDialog(win, scheme, data)
        });
      });
      win._find('UsersPasswd').on('click', function() {
        _action(function(data) {
          showDialog(win, scheme, null, true)
        });
      });
    },

    save: function(win, scheme, settings, wm) {
    }
  };

  /////////////////////////////////////////////////////////////////////////////
  // EXPORTS
  /////////////////////////////////////////////////////////////////////////////

  OSjs.Applications = OSjs.Applications || {};
  OSjs.Applications.ApplicationSettings = OSjs.Applications.ApplicationSettings || {};
  OSjs.Applications.ApplicationSettings.Modules = OSjs.Applications.ApplicationSettings.Modules || {};
  OSjs.Applications.ApplicationSettings.Modules.Users = module;

})(OSjs.Core.Application, OSjs.Core.Window, OSjs.Utils, OSjs.API, OSjs.User, OSjs.GUI);
