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
      'Toggle tools toolbar' : 'Бар с инструменти',
      'Toggle layers toolbar' : 'Бар с слоеве',
      'Layer' : 'Слой',
      'Effect' : 'Ефект',
      'Flip Horizontally' : 'Обърни хоризонтално',
      'Flip Vertically' : 'Обърни вертикално',
      'Foreground' : 'Преден фон',
      'Bakgrunn' : 'Заден фон',
      'Foreground (Fill) Color' : 'Преден фон (Запълни) цвят',
      'Background (Stroke) Color' : 'Заден фон цвят',
      'Line join' : 'Съединяване на линии',
      'Line width' : 'Широчина на линия',
      'Toggle Stroke' : 'Превключване на удър',
      'Enable stroke' : 'Включи удър',
      'Round' : 'Кръгъл',
      'Miter' : 'Митра',
      'Bevel' : 'Откос',
      'Stroked' : 'Ударен',
      'No stroke' : 'Без удър',

      'Pointer' : 'Стрелка',
      'Move active layer' : 'Премести активен слой',

      'Picker' : 'берач',
      'LMB: Pick foreground-, RMB: Pick background color' : 'LMB: изберете цвят за преден план, RMB: изберете цвят за фон',

      'Pencil' : 'Молив',
      'LMB/RMB: Draw with fg/bg color' : 'LMB/RMB: рисувай с fg/bg цвят',
      'Path' : 'Път',

      'Square/Rectangle' : 'Квадрат/Правоъгъкник',
      'LMB/RMB: Draw with fg/bg color, SHIFT: Draw rectangle' : 'LMB/RMB: рисувай с fb/bg цвят, SHIFT: нарисувай правоъгълник',

      'Circle/Ellipse' : 'Кръг/Елипса',
      'LMB/RMB: Draw with fg/bg color, SHIFT: Draw ellipse' : 'LMB/RMB: рисувай с fb/bg цвят, SHIFT: нарисувай елипса',

      'Blur' : 'Замъгли',
      'Noise' : 'Шум',
      'Invert colors' : 'Инвертирай цветове',
      'Grayscale' : 'Черно-бяло',
      'Sharpen' : 'Острота',
      'Simple Blur' : 'Опростено замъгляване',

      'Radius' : 'Радиус',
      'Iterations' : 'Повторения'
    },
    de_DE : {
      'Toggle tools toolbar' : 'Tools Toolbar',
      'Toggle layers toolbar' : 'Ebenen Toolbar',
      'Layer' : 'Ebene',
      'Effect' : 'Effekt',
      'Flip Horizontally' : 'Horizontal spiegeln',
      'Flip Vertically' : 'Vertikal spiegeln',
      'Foreground' : 'Vordergrund',
      'Bakgrunn' : 'Hintergrund',
      'Foreground (Fill) Color' : 'Vordergrund (Füll-) Farbe',
      'Background (Stroke) Color' : 'Hintergrund (Streich-) Farbe',
      'Line join' : 'Linienverbindung',
      'Line width' : 'Linienbreite',
      'Toggle Stroke' : 'Streichen',
      'Enable stroke' : 'Streichen aktivieren',
      'Round' : 'Runde',
      'Miter' : 'Live',
      'Bevel' : 'Schräge',
      'Stroked' : 'Gestrichen',
      'No stroke' : 'Nicht gestrichen',

      'Pointer' : 'Zeiger',
      'Move active layer' : 'Bewege aktive Ebene',

      'Picker' : 'Wähler',
      'LMB: Pick foreground-, RMB: Pick background color' : 'LMB: wähle Vordergrundfarbe, RMB: wähle Hintergrundfarbe',

      'Pencil' : 'Stift',
      'LMB/RMB: Draw with fg/bg color' : 'LMB/RMB: Zeichnen mit fg/bg Farbe',
      'Path' : 'Pfad',

      'Square/Rectangle' : 'Quadrat/Rechteck',
      'LMB/RMB: Draw with fg/bg color, SHIFT: Draw rectangle' : 'LMB/RMB: Zeichnen mit fb/bg Farbe, SHIFT: Rechteck zeichnen',

      'Circle/Ellipse' : 'Kreis/Ellipse',
      'LMB/RMB: Draw with fg/bg color, SHIFT: Draw ellipse' : 'LMB/RMB: Zeichnen mit fb/bg Farbe, SHIFT: Ellipse zeichnen',

      'Blur' : 'Weichzeichner (Blur)',
      'Noise' : 'Rauschen',
      'Invert colors' : 'Farben invertieren',
      'Grayscale' : 'Graustufen',
      'Sharpen' : 'Schärfen',
      'Simple Blur' : 'Einfacher Weichzeichner (Blur)',

      'Radius' : 'Radius',
      'Iterations' : 'Iterationen'
    },
    es_ES : {
      'Toggle tools toolbar' : 'Mostrar/ocultar la barra de herramientas de utilidades',
      'Toggle layers toolbar' : 'Mostrar/ocultar la barra de herramientas de capas',
      'Layer' : 'Capa',
      'Effect' : 'Efecto',
      'Flip Horizontally' : 'Girar horizontalmente',
      'Flip Vertically' : 'Girar verticalmente',
      'Foreground' : 'Primer plano',
      'Bakgrunn' : 'Fondo',
      'Foreground (Fill) Color' : 'Color de primer plano (relleno)',
      'Background (Stroke) Color' : 'Color de de fondo (contorno)',
      'Line join' : 'Terminación de línea',
      'Line width' : 'Ancho de línea',
      'Toggle Stroke' : 'Activar/Desactivar trazado',
      'Enable stroke' : 'Activar trazado',
      'Round' : 'Curvo',
      'Miter' : 'En ángulo',
      'Bevel' : 'Biselado',
      'Stroked' : 'Trazado',
      'No stroke' : 'Sin trazado',

      'Pointer' : 'Puntero',
      'Move active layer' : 'Mover la capa activa',

      'Picker' : 'Selector',
      'LMB: Pick foreground-, RMB: Pick background color' : 'LMB: Establecer el color de primer plano, RMB: Establecer el color de fondo',

      'Pencil' : 'Lápiz',
      'LMB/RMB: Draw with fg/bg color' : 'LMB/RMB: Dibujar con el color de fondo/de primer plano',
      'Path' : 'Ruta',

      'Square/Rectangle' : 'Cuadrado/Rectángulo',
      'LMB/RMB: Draw with fg/bg color, SHIFT: Draw rectangle' : 'LMB/RMB: Zeichnen mit fb/bg Farbe, SHIFT: Rechteck zeichnen',

      'Circle/Ellipse' : 'Kreis/Ellipse',
      'LMB/RMB: Draw with fg/bg color, SHIFT: Draw ellipse' : 'LMB/RMB: Dibujar con el color de fondo/de primer plano, SHIFT: Dibujar una elipse',

      'Blur' : 'Desenfoque',
      'Noise' : 'Ruido',
      'Invert colors' : 'Invertir colores',
      'Grayscale' : 'Escala de grises',
      'Sharpen' : 'Afilar',
      'Simple Blur' : 'Desenfoque simple',

      'Radius' : 'Radio',
      'Iterations' : 'Iteraciones'
    },
    fr_FR : {
      'Toggle tools toolbar' : 'Afficher la barre d\'outils',
      'Toggle layers toolbar' : 'Afficher la barre des calques',
      'Layer' : 'Calque',
      'Effect' : 'Effet',
      'Flip Horizontally' : 'Pivoter horizontalement',
      'Flip Vertically' : 'Pivoter verticalement',
      'Foreground' : 'Avant-plan',
      'Background' : 'Arrière-plan',
      'Foreground (Fill) Color' : 'Couleur de l\'avant-plan (remplissage)',
      'Background (Stroke) Color' : 'Couleur de l\'arrière-plan (trait)',
      'Line join' : 'Jointure',
      'Line width' : 'Taille de la ligne',
      'Toggle Stroke' : 'Afficher les traits',
      'Enable stroke' : 'Activer les traits',
      'Round' : 'Arrondi',
      'Miter' : 'Pointu',
      'Bevel' : 'Biseauté',
      'Stroked' : 'Barré',
      'No stroke' : 'Non barré',

      'Pointer' : 'Pointeur',
      'Move active layer' : 'Déplacer le calque actif',

      'Picker' : 'Sélecteur',
      'LMB: Pick foreground-, RMB: Pick background color' : 'Clic gauche: sélectionne la couleur de l\'avant-plan, clic droit: sélectionne la couleur de l\'arrière-plan',

      'Pencil' : 'Pinceau',
      'LMB: Use foreground-, RMB: Use background color' : 'Clic gauche: utilise la couleur d\'avant-plan, clic droit : utilise la couleur d\'arrière-plan',
      'Path' : 'Chemin',

      'Square/Rectangle' : 'Carré/Rectangle',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle rectangle/square mode' : 'Clic gauche: utilise la couleur d\'avant-plan, clic droit: utilise la couleur d\'arrière-plan, SHIFT: affiche le mode rectangle',

      'Circle/Ellipse' : 'Cercle/Ellipse',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle circle/ellipse mode' : 'Clic gauche: utilise la couleur d\'avant-plan, clic droit: utilise la couleur d\'arrière-plan, SHIFT: affiche le mode ellipse',

      'LMB: Fill with foreground-, RMB: Fill with background color': 'Clic gauche: remplir avec la couleur d\'avant-plan, clic droit: remplir avec la couleur d\'arrière-plan',
      'Set foreground color': 'Définir la couleur d\'avant-plan',
      'Set background color': 'Définir la couleur d\'arrière-plan',

      'Blur' : 'Flou',
      'Noise' : 'Bruit',
      'Invert colors' : 'Inverser les couleurs',
      'Grayscale' : 'Niveau de gris',
      'Sharpen' : 'Netteté',
      'Simple Blur' : 'Flou simple',

      'Radius' : 'Rayon',
      'Iterations' : 'Itérations'
    },
    ar_DZ : {
      'Toggle tools toolbar' : 'إظهار شريط الأدوات',
      'Toggle layers toolbar' : 'إظهار شريط الشفائف',
      'Layer' : 'شفيفة',
      'Effect' : 'تأثير',
      'Flip Horizontally' : 'دوران أفقي',
      'Flip Vertically' : 'دوران عمودي',
      'Foreground' : 'الأمامي',
      'Background' : 'الخلفية',
      'Foreground (Fill) Color' : 'اللون الأمامي (التعبئة)',
      'Background (Stroke) Color' : 'لون الخلفية (الخط)',
      'Line join' : 'مفصل خطي',
      'Line width' : 'عرض الخط',
      'Toggle Stroke' : 'إظهار الخطوط',
      'Enable stroke' : 'تفعيل الخطوط',
      'Round' : 'دائري',
      'Miter' : 'مذبب',
      'Bevel' : 'شطف',
      'Stroked' : 'مشطوب',
      'No stroke' : 'بدون خط',

      'Pointer' : 'المؤشر',
      'Move active layer' : 'نقل الشفيفة المفعلة',

      'Picker' : 'المرشح',
      'LMB: Pick foreground-, RMB: Pick background color' : 'ضغط باليسار: إختيار اللون الأمامي, ضغط باليمين: إختيار لون الخلفية',

      'Pencil' : 'القلم',
      'LMB: Use foreground-, RMB: Use background color' : 'ضغط باليسار: إستعمال اللون الأمامي, ضغط باليمين: إستعمال لون الخلفية',
      'Path' : 'المسار',

      'Square/Rectangle' : 'مربع/مستطيل',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle rectangle/square mode' : 'ضغط باليسار: إستعمال اللون الأمامي, ضغط باليمين: إستعمال لون الخلفية، SHIFT : تحويل بين مربع/مستطيل',

      'Circle/Ellipse' : 'دائرة/قطع ناقص',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle circle/ellipse mode' : 'ضغط باليسار: إستعمال اللون الأمامي, ضغط باليمين: إستعمال لون الخلفية، SHIFT : تحويل بين دائرة/قطع ناقص',

      'LMB: Fill with foreground-, RMB: Fill with background color': 'ضغط باليسار: ملئ باللون الأمامي, ضغط باليمين: ملئ بلون الخلفية',
      'Set foreground color': 'إستعمال كلون أمامي',
      'Set background color': 'إستعمال كلون للخلفية',

      'Blur' : 'ضبابي',
      'Noise' : 'ضجيج',
      'Invert colors' : 'عكس الألوان',
      'Grayscale' : 'مستوى الرمادي',
      'Sharpen' : 'صفاء',
      'Simple Blur' : 'ضباب عادي',

      'Radius' : 'القطر',
      'Iterations' : 'عدد العمليات'
    },
    it_IT : {
      'Toggle tools toolbar' : 'Mostra la barra strumenti',
      'Toggle layers toolbar' : 'Mostra la barra dei livelli',
      'Layer' : 'Livello',
      'Effect' : 'Effetto',
      'Flip Horizontally' : 'Specchia orizzontalmente',
      'Flip Vertically' : 'Specchia verticalmente',
      'Foreground' : 'Primopiano',
      'Background' : 'Sfondo',
      'Foreground (Fill) Color' : 'Colore in primopiano (Riempimento)',
      'Background (Stroke) Color' : 'Colore di sfondo (Tracciato)',
      'Line join' : 'Congiungi linea',
      'Line width' : 'Lunghezza linea',
      'Toggle Stroke' : 'Mostra tracciato',
      'Enable stroke' : 'Abilita tracciato',
      'Round' : 'Arrotonda',
      'Miter' : 'Miter',
      'Bevel' : 'Smussatura',
      'Stroked' : 'Tracciato',
      'No stroke' : 'Nessun tracciato',

      'Pointer' : 'Puntatore',
      'Move active layer' : 'Sposta livello attivo',

      'Picker' : 'Selettore',
      'LMB: Pick foreground-, RMB: Pick background color' : 'LMB: Imposta colore primopiano, RMB: Imposta colore di sfondo',

      'Pencil' : 'Matita',
      'LMB/RMB: Draw with fg/bg color' : 'LMB/RMB: Disegna con colori fg/bg',
      'Path' : 'Percorso',

      'Square/Rectangle' : 'Quadrato/Rettangolo',
      'LMB/RMB: Draw with fg/bg color, SHIFT: Draw rectangle' : 'LMB/RMB: Disegna con colori di fg/bg, SHIFT: Disegna rettangolo',

      'Circle/Ellipse' : 'Cerchio/Ellisse',
      'LMB/RMB: Draw with fg/bg color, SHIFT: Draw ellipse' : 'LMB/RMB: Disegna con colori di fg/bg, SHIFT: Disegna ellisse',

      'Blur' : 'Sfoca',
      'Noise' : 'Disturbo',
      'Invert colors' : 'Inverti colori',
      'Grayscale' : 'Scala di grigi',
      'Sharpen' : 'Intensifica',
      'Simple Blur' : 'Sfocatura leggera',

      'Radius' : 'Raggio',
      'Iterations' : 'Ripetizioni'
    },
    ko_KR : {
      'Toggle tools toolbar' : '도구 툴바 켜기/끄기',
      'Toggle layers toolbar' : '레이어 툴바 켜기/끄기',
      'Layer' : '레이어',
      'Effect' : '효과',
      'Flip Horizontally' : '수평으로 뒤집기',
      'Flip Vertically' : '수직으로 뒤집기',
      'Foreground' : '전경',
      'Background' : '배경',
      'Foreground (Fill) Color' : '전경색(채우기)',
      'Background (Stroke) Color' : '배경색(칠하기)',
      'Line join' : '선 종류',
      'Line width' : '선 굵기',
      'Toggle Stroke' : '선 활성화',
      'Enable stroke' : '선 그리기',
      'Round' : '둥글게',
      'Miter' : '기울이기',
      'Bevel' : '비스듬히',
      'Stroked' : '선 보이기',
      'No stroke' : '선없음',

      'Pointer' : '포인터',
      'Move active layer' : '활성 레이어 옮기기',

      'Picker' : '색상 추출',
      'LMB: Pick foreground-, RMB: Pick background color' : '왼쪽 마우스 버튼: 전경색, 오른쪽 마우스 버튼 : 배경색',

      'Pencil' : '연필',
      'LMB: Use foreground-, RMB: Use background color' : '왼쪽 마우스/오른쪽 마우스 버튼 전경/배경색 그리기',
      'Path' : '경로',

      'Square/Rectangle' : '정사각형/직사각형',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle rectangle/square mode' : '왼쪽/오른쪽 마우스 버튼 전경/배경색 그리기, SHIFT: 직사각형 그리기',

      'Circle/Ellipse' : '원/타원',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle circle/ellipse mode' : '왼쪽/오른쪽 마우스 버튼 전경/배경색 그리기, SHIFT: 타원 그리기',

      'Blur' : '블러',
      'Noise' : '노이즈',
      'Invert colors' : '반전',
      'Grayscale' : '흑백',
      'Sharpen' : '샤픈',
      'Simple Blur' : '약한 블러',

      'Radius' : '반경',
      'Iterations' : '반복',

      'LMB: Fill with foreground-, RMB: Fill with background color': '왼쪽 마우스/오른쪽 마우스 버튼 전경/배경색 칠하기',
      'Set {0} color': '{0}색을 선택'
    },
    nl_NL : {
      'Toggle tools toolbar' : 'Toolbar gereedschappen',
      'Toggle layers toolbar' : 'Toolbar lagen',
      'Layer' : 'Laag',
      'Effect' : 'Effecten',
      'Flip Horizontally' : 'Horizontaal spiegelen',
      'Flip Vertically' : 'Verticaal spiegelen',
      'Foreground' : 'Voorgrond',
      'Background' : 'Achtergrond',
      'Foreground (Fill) Color' : 'Voorgrond (vul) kleur',
      'Background (Stroke) Color' : 'Achtergrond (penseel-) kleur',
      'Line join' : 'Lijnverbinding',
      'Line width' : 'Lijnbreedte',
      'Toggle Stroke' : 'Penseel streek',
      'Enable stroke' : 'Penseel activeren',
      'Round' : 'Rond',
      'Miter' : 'Live',
      'Bevel' : 'Schuin',
      'Stroked' : 'Gestreken',
      'No stroke' : 'Geen penseel streken',

      'Pointer' : 'Aanwijzer',
      'Move active layer' : 'Verplaats de actieve laag',

      'Picker' : 'Kiezer',
      'LMB: Pick foreground-, RMB: Pick background color' : 'LMB: Kies voorgrond-kleur, RMB: Kies achtergrondkleur',

      'Pencil' : 'Penseel',
      'LMB: Use foreground-, RMB: Use background color' : 'LMB: Teken met voorgrond-, RMB: Teken met achtergrond-kleur',
      'Path' : 'Pfad',

      'Square/Rectangle' : 'Vierkant/rechthoek',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle rectangle/square mode' : 'LMB/RMB: Teken met voor- en achtergrondkleur, SHIFT: Rechthoek tekenen',

      'Circle/Ellipse' : 'Cirkel/elipse',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle circle/ellipse mode' : 'LMB/RMB: Teken met voor- en achtergrondkleur, SHIFT: Elipse tekenen',

      'Blur' : 'Vervagen (Blur)',
      'Noise' : 'Ruis',
      'Invert colors' : 'Kleuren inverteren',
      'Grayscale' : 'Grijstinten',
      'Sharpen' : 'Verscherpen',
      'Simple Blur' : 'Eenvoudig vervagen (Blur)',

      'Radius' : 'Radius',
      'Iterations' : 'Herhalingen'
    },
    no_NO : {
      'Toggle tools toolbar' : 'Svitsj verktøylinje',
      'Toggle layers toolbar' : 'Svitsj lag-verktøylinje',
      'Layer' : 'Lag',
      'Effect' : 'Effekt',
      'Flip Horizontally' : 'Flipp Horisontalt',
      'Flip Vertically' : 'Flipp Vertikalt',
      'Foreground' : 'Forgrunn',
      'Background' : 'Bakgrunn',
      'Foreground (Fill) Color' : 'Forgrunn (Fyll) Farge',
      'Background (Stroke) Color' : 'Bakgrunn (Strøk) Farge',
      'Line join' : 'Linje Knytting',
      'Line width' : 'Linje Bredde',
      'Toggle Stroke' : 'Svitsj strøk',
      'Enable stroke' : 'Skru på strøk',
      'Round' : 'Rund',
      'Miter' : 'Skjev',
      'Bevel' : 'Kantet',
      'Stroked' : 'Strøk På',
      'No stroke' : 'Strøk Av',

      'Pointer' : 'Peker',
      'Move active layer' : 'Flytt aktivt lag',

      'Picker' : 'Plukker',
      'LMB: Pick foreground-, RMB: Pick background color' : 'LMB: sett bg farge, RMB: sett fg farge',

      'Pencil' : 'Penn',
      'LMB: Use foreground-, RMB: Use background color' : 'LMB/RMB: Tegn med fg/bg farge',
      'Path' : 'Sti',

      'Square/Rectangle' : 'Firkant/Rektangel',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle rectangle/square mode' : 'LMB/RMB: Tegn med fg/bg farge, SHIFT: Tegn rektangel',

      'Circle/Ellipse' : 'Sirkel/Ellipse',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle circle/ellipse mode' : 'LMB/RMB: Tegn med fg/bg farge, SHIFT: Tegn ellipse',

      'Blur' : 'Klatte (Blur)',
      'Noise' : 'Støy',
      'Invert colors' : 'Inverter farger',
      'Grayscale' : 'Gråskala',
      'Sharpen' : 'Skarpgjør',
      'Simple Blur' : 'Simpel Klatte (Blur)',

      'Radius' : 'Radius',
      'Iterations' : 'Itereringer'
    },
    pl_PL : {
      'Toggle tools toolbar' : 'Przełącz Pasek narzędzi',
      'Toggle layers toolbar' : 'Przełącz Pasek warstw',
      'Layer' : 'Warstwy',
      'Effect' : 'Efekty',
      'Flip Horizontally' : 'Przerzuć w poziomie',
      'Flip Vertically' : 'Przerzuć w pionie',
      'Foreground' : 'Pierwszy plan',
      'Bakgrunn' : 'Tło',
      'Foreground (Fill) Color' : 'Kolor pierwszoplanowy (Wypełnienie)',
      'Background (Stroke) Color' : 'Kolor tła (Wycinanie)',
      'Line join' : 'Rodzaj lini',
      'Line width' : 'Grubość',
      'Toggle Stroke' : 'Włącz/wyłącz obramowanie',
      'Enable stroke' : 'Włącz obramowanie',
      'Round' : 'Zwykłe',
      'Miter' : 'Paski',
      'Bevel' : 'Kątownik',
      'Stroked' : 'Obranowanie',
      'No stroke' : 'Bez obramowania',

      'Pointer' : 'Wskaźnik',
      'Move active layer' : 'Przenieś aktywne warstwy',

      'Picker' : 'Wybór',
      'LMB: Pick foreground-, RMB: Pick background color' : 'LMB: Wstaw kolor bg, RMB: ustaw kolor fg',

      'Pencil' : 'Ołówek',
      'LMB: Use foreground-, RMB: Use background color' : 'Maluj w kolorze fg/bg',
      'Path' : 'Ścieżka',

      'Square/Rectangle' : 'Kwadratowe / prostokątne',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle rectangle/square mode' : 'LMB/RMB: Maluj w kolorze fb/bg, SHIFT: Narysuj prostokąt',

      'Circle/Ellipse' : 'Koło / Elipsa',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle circle/ellipse mode' : 'LMB/RMB: Maluj w kolorze fb/bg, SHIFT: Narysuj elipse',

      'Blur' : 'Blur',
      'Noise' : 'Szum',
      'Invert colors' : 'Odwróc kolory',
      'Grayscale' : 'Skala szaroścu',
      'Sharpen' : 'Zaostrzone',
      'Simple Blur' : 'Łatwy Blur',

      'Radius' : 'Promień',
      'Iterations' : 'Powtórzenia'
    },
    ru_RU : {
      'Toggle tools toolbar' : 'Панель инструментов',
      'Toggle layers toolbar' : 'Панель слоев',
      'Layer' : 'Слой',
      'Effect' : 'Эффекты',
      'Flip Horizontally' : 'Отразить горизонтально',
      'Flip Vertically' : 'Отразить вертикально',
      'Foreground' : 'Передний план',
      'Bakgrunn' : 'Фон',
      'Foreground (Fill) Color' : 'Передний план (Заливка) цвет',
      'Background (Stroke) Color' : 'Фоновый (Обводка) цвет',
      'Line join' : 'Замкнутая линия',
      'Line width' : 'Ширина линии',
      'Toggle Stroke' : 'Вкл/выкл обводку',
      'Enable stroke' : 'Включить обводку',
      'Round' : 'Закругленный',
      'Miter' : 'Прямой',
      'Bevel' : 'Скошенный',
      'Stroked' : 'С обводкой',
      'No stroke' : 'Без обводки',

      'Pointer' : 'Указатель',
      'Move active layer' : 'Перемещает активный слой',

      'Picker' : 'Пипетка',
      'LMB: Pick foreground-, RMB: Pick background color' : 'ЛКМ: устананавливает первичный цвет, ПКМ: устанавливает вторичный(фоновый) цвет',

      'Pencil' : 'Карандаш',
      'LMB: Use foreground-, RMB: Use background color' : 'ЛКМ/ПКМ: Рисует первичным/вторичным цветом',
      'Path' : 'Прямая',

      'Square/Rectangle' : 'Квадрат/Прямоугольник',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle rectangle/square mode' : 'ЛКМ/ПКМ: рисует первичным/вторичным цветом квадрат, SHIFT: нарисовать прямоуголник',

      'Circle/Ellipse' : 'Круг/Эллипс',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle circle/ellipse mode' : 'ЛКМ/ПКМ: рисует первичным/вторичным цветом круг, SHIFT: нарисовать эллипс',

      'Blur' : 'Размытие (Blur)',
      'Noise' : 'Шум',
      'Invert colors' : 'Инвертировать цвета',
      'Grayscale' : 'Обесцветить',
      'Sharpen' : 'Сточить',
      'Simple Blur' : 'Простое размытие (Blur)',

      'Radius' : 'Радиус',
      'Iterations' : 'Итерации'
    },
    sk_SK : {
      'Toggle tools toolbar' : 'Zobraz panel nástrojov',
      'Toggle layers toolbar' : 'Zobraz vrstvy',
      'Layer' : 'Vrstvy',
      'Effect' : 'Efekty',
      'Flip Horizontally' : 'Transformuj horizontálne',
      'Flip Vertically' : 'Transformuj vertikálne',
      'Foreground' : 'Popredie',
      'Bakgrunn' : 'Pozadie',
      'Line join' : 'Typ čiary',
      'Line width' : 'Šírka čiary',
      'Toggle Stroke' : 'Zapnúť orámovanie',
      'Enable stroke' : 'Orámovanie',
      'Round' : 'Okrúhly',
      'Miter' : 'Naklonený',
      'Bevel' : 'Šikmý',
      'Stroked' : 'Orámovaný',
      'No stroke' : 'Bez orámovania',

      'Pointer' : 'Ukazovateľ',
      'Move active layer' : 'Presuň aktívnu vrstvu',

      'Picker' : 'Kurzor',
      'LMB: Pick foreground-, RMB: Pick background color' : 'LMB: nastav farbu pozadia, RMB: nastav farbu popredia',

      'Pencil' : 'Ceruzka',
      'LMB: Use foreground-, RMB: Use background color' : 'Maľuj farbou fg/bg',
      'Path' : 'Cesta',

      'Square/Rectangle' : 'Štvorec / Obdĺžnik',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle rectangle/square mode' : 'LMB/RMB: Maľuj farbou fb/bg, SHIFT: Obdĺžnik',

      'Circle/Ellipse' : 'Kruh / Elipsa',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle circle/ellipse mode' : 'LMB/RMB: Maľuj farbou fb/bg, SHIFT: Elipsa',

      'Blur' : 'Rozmazať',
      'Noise' : 'Šum',
      'Invert colors' : 'Invertovať farby',
      'Grayscale' : 'Odtiene šedej',
      'Sharpen' : 'Zostriť',
      'Simple Blur' : 'Jednoduchý Blur',

      'Radius' : 'Rádius',
      'Iterations' : 'Iterácie'
    },
    tr_TR : {
      'Toggle tools toolbar' : 'araç çubugu değiştirme araçları',  //yanlış olabilir
      'Toggle layers toolbar' : 'Araç çubugu katmanı değiştirme',
      'Layer' : 'Katman',
      'Effect' : 'efekt',
      'Flip Horizontally' : 'Yatay çevir',
      'Flip Vertically' : 'Dikey çevir',
      'Foreground' : 'Önplana al',
      'Background' : 'Arkaplana al',
      'Foreground (Fill) Color' : 'Önplan rengi',
      'Background (Stroke) Color' : 'Arkaplan rengi',
      'Line join' : 'Çizgi bitişimi',
      'Line width' : 'Çizgi genişliği',
      'Toggle Stroke' : 'vuruşu değiştir',  //eklenecek
      'Enable stroke' : 'vuruş aktif',  //eklenecek
      'Round' : 'yuvarlamak',
      'Miter' : 'gönye',
      'Bevel' : 'eğmek',
      'Stroked' : 'Stroked',
      'No stroke' : 'No stroke',

      'Pointer' : 'işaretçi',
      'Move active layer' : 'hareket eden katman',

      'Picker' : 'toplayıcı',
      'LMB: Pick foreground-, RMB: Pick background color' : 'LMB: fg rengi ayarla, RMB: gb rengi ayarla',

      'Pencil' : 'kalem',
      'LMB: Use foreground-, RMB: Use background color' : 'LMB/RMB:fg/bg rengi ile çiz',
      'Path' : 'yol',

      'Square/Rectangle' : 'kare/üçgen',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle rectangle/square mode' : 'LMB/RMB: fg/bg renkleri ile çiz , SHIFT: üçgen çiz',

      'Circle/Ellipse' : 'dair/elips',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle circle/ellipse mode' : 'LMB/RMB: fb/bg ile çiz , SHIFT: elips çiz',

      'Blur' : 'Bulanık',
      'Noise' : 'gürültü',
      'Invert colors' : 'renkleri tersine çevir',
      'Grayscale' : 'gri ton',
      'Sharpen' : 'keskinleştirmek',
      'Simple Blur' : 'sade Bulanık',

      'Radius' : 'yarıçap',
      'Iterations' : 'yineleme'
    },
    vi_VN : {
      'Toggle tools toolbar' : 'Công cụ bật tắt thanh công cụ',
      'Toggle layers toolbar' : 'Bật tắt cửa sổ layer',
      'Layer' : 'Lớp',
      'Effect' : 'Hiệu ứng',
      'Flip Horizontally' : 'Lật ngang',
      'Flip Vertically' : 'Lật theo chiều dọc',
      'Foreground' : 'Nền trước',
      'Background' : 'Nền dưới',
      'Foreground (Fill) Color' : 'Màu nền trước',
      'Background (Stroke) Color' : 'Màu nền dưới',
      'Line join' : 'Ghép đoạn thằng',
      'Line width' : 'Độ rộng dòng',
      'Toggle Stroke' : 'Bật tắt nét',
      'Enable stroke' : 'Bật nét',
      'Round' : 'Tròn',
      'Miter' : 'Góc',
      'Bevel' : 'Góc xiên',
      'Stroked' : 'Vuốt',
      'No stroke' : 'Không vuốt',

      'Pointer' : 'Con trỏ',
      'Move active layer' : 'Di chuyển layer đang chọn',

      'Picker' : 'Bảng chọn',
      'LMB: Pick foreground-, RMB: Pick background color' : 'LMB: chọn màu fg, RMB: chọn màu gb',

      'Pencil' : 'Bút chì',
      'LMB: Use foreground-, RMB: Use background color' : 'LMB/RMB: vẽ với màu fg/bg',
      'Path' : 'Đường',

      'Square/Rectangle' : 'Firkant/Rektangel',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle rectangle/square mode' : 'LMB/RMB: Tegn med fg/bg farge, SHIFT: Tegn rektangel',

      'Circle/Ellipse' : 'Vuông / chữ nhật',
      'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle circle/ellipse mode' : 'LMB/RMB: vẽ với màu fg/bg, SHIFT: Vẽ ê-líp',

      'Blur' : 'Làm mờ',
      'Noise' : 'Làm nhiễu',
      'Invert colors' : 'Nghịch đảo màu',
      'Grayscale' : 'Độ xám',
      'Sharpen' : 'Làm sắc nét',
      'Simple Blur' : 'Làm mờ đơn giản',

      'Radius' : 'Bán kính',
      'Iterations' : 'Lặp đi lặp lại'
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
  OSjs.Applications.ApplicationDraw = OSjs.Applications.ApplicationDraw || {};
  OSjs.Applications.ApplicationDraw._ = _;

})(OSjs.Helpers.DefaultApplication, OSjs.Core.Window, OSjs.GUI, OSjs.Dialogs, OSjs.Utils, OSjs.API, OSjs.VFS);

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
(function(DefaultApplication, DefaultApplicationWindow, Application, Window, Utils, API, VFS, GUI) {
  'use strict';

  var DEFAULT_WIDTH = 1024;
  var DEFAULT_HEIGHT = 768;

  var tools = {
    pointer: {
      statusText: ''
    },
    picker: {
      statusText: 'LMB: Pick foreground-, RMB: Pick background color'
    },
    bucket: {
      statusText: 'LMB: Fill with foreground-, RMB: Fill with background color'
    },
    pencil: {
      statusText: 'LMB: Use foreground-, RMB: Use background color'
    },
    path: {
      statusText: 'LMB: Use foreground-, RMB: Use background color'
    },
    rectangle: {
      statusText: 'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle rectangle/square mode'
    },
    circle: {
      statusText: 'LMB: Use foreground-, RMB: Use background color. SHIFT: Toggle circle/ellipse mode'
    }
  };
  var toolEvents = {
    pointer: {
    },

    picker: (function() {
      var imageData;

      function pick(ev, args) {
        var value = '#000000';
        var t = (ev.shiftKey || ev.button > 0) ? 'background' : 'foreground';

        if ( !imageData ) {
          imageData = args.ctx.getImageData(0, 0, args.canvas.width, args.canvas.height).data;
        }

        var index = ((args.pos.x + args.pos.y * args.canvas.width) * 4);
        try {
          value = Utils.convertToHEX({
            r:imageData[index + 0],
            g:imageData[index + 1],
            b:imageData[index + 2],
            a:imageData[index + 3]
          });
        } catch ( e ) {
        }

        args.win.setToolProperty(t, value);
      }

      return {
        mousedown: pick,
        mousemove: pick,
        mouseup: function(ev, pos, canvas, ctx, win) {
          imageData = null;
        }
      };
    })(),

    bucket: {
      mousedown: function(ev, args) {
        var t = (ev.shiftKey || ev.button > 0) ? 'background' : 'foreground';
        args.ctx.fillStyle = args.win.tool[t];
        args.ctx.fillRect(0, 0, args.canvas.width, args.canvas.height);
      }
    },

    pencil: {
      mousedown: function(ev, args) {
        var t = (ev.shiftKey || ev.button > 0) ? 'background' : 'foreground';
        args.ctx.strokeStyle = args.win.tool[t];
      },
      mousemove: function(ev, args) {
        args.ctx.beginPath();
        args.ctx.moveTo(args.pos.x - 1, args.pos.y);
        args.ctx.lineTo(args.pos.x, args.pos.y);
        args.ctx.closePath();
        args.ctx.stroke();
      }
    },

    path: {
      mousemove: function(ev, args) {
        if ( args.tmpContext ) {
          args.tmpContext.clearRect(0, 0, args.tmpCanvas.width, args.tmpCanvas.height);
          args.tmpContext.beginPath();
          args.tmpContext.moveTo(args.start.x, args.start.y);
          args.tmpContext.lineTo(args.pos.x, args.pos.y);
          args.tmpContext.closePath();
          args.tmpContext.stroke();
        }
      }
    },

    rectangle: {
      mousedown: function(ev, args) {
        args.tmpContext.fillStyle = (ev.button > 0) ? args.win.tool.background : args.win.tool.foreground;
        args.tmpContext.strokeStyle = (ev.button <= 0) ? args.win.tool.background : args.win.tool.foreground;
      },
      mousemove: function(ev, args) {
        var x, y, w, h;

        if ( ev.shiftKey ) {
          x = Math.min(args.pos.x, args.start.x);
          y = Math.min(args.pos.y, args.start.y);
          w = Math.abs(args.pos.x - args.start.x);
          h = Math.abs(args.pos.y - args.start.y);
        } else {
          x = args.start.x; //Math.min(args.pos.x, args.start.x);
          y = args.start.y; //Math.min(args.pos.y, args.start.y);
          w = Math.abs(args.pos.x - args.start.x) * (args.pos.x < args.start.x ? -1 : 1);
          h = Math.abs(w) * (args.pos.y < args.start.y ? -1 : 1);
        }

        args.tmpContext.clearRect(0, 0, args.tmpCanvas.width, args.tmpCanvas.height);
        if ( w && h ) {
          if ( args.win.tool.lineStroke ) {
            args.tmpContext.strokeRect(x, y, w, h);
          }
          args.tmpContext.fillRect(x, y, w, h);
        }
      }
    },

    circle: {
      mousedown: function(ev, args) {
        args.tmpContext.fillStyle = (ev.button > 0) ? args.win.tool.background : args.win.tool.foreground;
        args.tmpContext.strokeStyle = (ev.button <= 0) ? args.win.tool.background : args.win.tool.foreground;
      },
      mousemove: function(ev, args) {
        if ( ev.shiftKey ) {
          var width = Math.abs(args.start.x - args.pos.x);
          var height = Math.abs(args.start.y - args.pos.y);

          args.tmpContext.clearRect(0, 0, args.tmpCanvas.width, args.tmpCanvas.height);
          if ( width > 0 && height > 0 ) {
            args.tmpContext.beginPath();
            args.tmpContext.moveTo(args.start.x, args.start.y - height * 2); // A1
            args.tmpContext.bezierCurveTo(
              args.start.x + width * 2, args.start.y - height * 2, // C1
              args.start.x + width * 2, args.start.y + height * 2, // C2
              args.start.x, args.start.y + height * 2); // A2

            args.tmpContext.bezierCurveTo(
              args.start.x - width * 2, args.start.y + height * 2, // C3
              args.start.x - width * 2, args.start.y - height * 2, // C4
              args.start.x, args.start.y - height * 2); // A1

            args.tmpContext.closePath();
            if ( args.win.tool.lineStroke ) {
              args.tmpContext.stroke();
            }
            args.tmpContext.fill();
          }
        } else {
          var x = Math.abs(args.start.x - args.pos.x);
          var y = Math.abs(args.start.y - args.pos.y);
          var r = Math.sqrt(Math.pow(x, 2) + Math.pow(y, 2));

          args.tmpContext.clearRect(0, 0, args.tmpCanvas.width, args.tmpCanvas.height);
          if ( r > 0 ) {
            args.tmpContext.beginPath();
            args.tmpContext.arc(args.start.x, args.start.y, r, 0, Math.PI * 2, true);
            args.tmpContext.closePath();

            if ( args.win.tool.lineStroke ) {
              args.tmpContext.stroke();
            }
            args.tmpContext.fill();
          }
        }
      }
    }
  };

  /////////////////////////////////////////////////////////////////////////////
  // WINDOWS
  /////////////////////////////////////////////////////////////////////////////

  var doTranslate = OSjs.Applications.ApplicationDraw._;

  function ApplicationDrawWindow(app, metadata, scheme, file) {
    DefaultApplicationWindow.apply(this, ['ApplicationDrawWindow', {
      icon: metadata.icon,
      title: metadata.name,
      allow_drop: true,
      min_width: 400,
      min_height: 450,
      width: 800,
      height: 450
    }, app, scheme, file]);

    this.tool = {
      name: 'pointer',
      background: '#ffffff',
      foreground: '#000000',
      lineJoin: 'round',
      lineWidth: 1,
      lineStroke: false
    };
  }

  ApplicationDrawWindow.prototype = Object.create(DefaultApplicationWindow.prototype);
  ApplicationDrawWindow.constructor = DefaultApplicationWindow.prototype;

  ApplicationDrawWindow.prototype.init = function(wm, app, scheme) {
    var root = DefaultApplicationWindow.prototype.init.apply(this, arguments);
    var self = this;

    // Load and set up scheme (GUI) here
    scheme.render(this, 'DrawWindow', root, null, null, {
      _: doTranslate
    });

    var statusbar = scheme.find(this, 'Statusbar');

    //
    // Canvas
    //
    var canvas = scheme.find(this, 'Canvas').querySelector('canvas');
    canvas.width = DEFAULT_WIDTH;
    canvas.height = DEFAULT_HEIGHT;

    var ctx = canvas.getContext('2d');

    var startPos = {x: 0, y: 0};
    var cpos = {x: 0, y: 0};
    var tmpTools = ['path', 'rectangle', 'circle'];
    var tmpCanvas, tmpContext;

    function createTempCanvas(ev) {
      tmpCanvas = document.createElement('canvas');
      tmpCanvas.width = canvas.width;
      tmpCanvas.height = canvas.height;
      tmpCanvas.style.position = 'absolute';
      tmpCanvas.style.top = '0px';
      tmpCanvas.style.left = '0px';
      tmpCanvas.style.zIndex = 9999999999;
      canvas.parentNode.appendChild(tmpCanvas);

      var t = (ev.shiftKey || ev.button > 0);

      tmpContext = tmpCanvas.getContext('2d');
      tmpContext.strokeStyle = t ? ctx.fillStyle : ctx.strokeStyle;
      tmpContext.fillStyle = t ? ctx.strokeStyle : ctx.fillStyle;
      tmpContext.lineWidth = ctx.lineWidth;
      tmpContext.lineJoin = ctx.lineJoin;
    }

    function removeTempCanvas() {
      Utils.$remove(tmpCanvas);
      tmpContext = null;
      tmpCanvas = null;
    }

    function toolAction(action, ev, pos, diff) {
      if ( action === 'down' ) {
        startPos = {x: pos.x, y: pos.y};

        removeTempCanvas();

        var elpos = Utils.$position(canvas);
        startPos.x = pos.x - elpos.left;
        startPos.y = pos.y - elpos.top;
        cpos = {x: startPos.x, y: startPos.y};

        ctx.strokeStyle = self.tool.foreground;
        ctx.fillStyle   = self.tool.background;
        ctx.lineWidth   = self.tool.lineWidth;
        ctx.lineJoin    = self.tool.lineJoin;

        if ( tmpTools.indexOf(self.tool.name) >= 0 ) {
          createTempCanvas(ev);
        }
      } else if ( action === 'move' ) {
        cpos.x = startPos.x + diff.x;
        cpos.y = startPos.y + diff.y;
      } else if ( action === 'up' ) {
        if ( tmpCanvas && ctx ) {
          ctx.drawImage(tmpCanvas, 0, 0);
        }
        removeTempCanvas();
        startPos = null;
      }

      if ( toolEvents[self.tool.name] && toolEvents[self.tool.name]['mouse' + action] ) {
        toolEvents[self.tool.name]['mouse' + action](ev, {
          pos: cpos,
          start: startPos,
          canvas: canvas,
          ctx: ctx,
          tmpContext: tmpContext,
          tmpCanvas: tmpCanvas,
          win: self
        });
      }
    }

    GUI.Helpers.createDrag(canvas, function(ev, pos) {
      toolAction('down', ev, pos);
    }, function(ev, diff, pos) {
      toolAction('move', ev, pos, diff);
    }, function(ev, pos) {
      toolAction('up', ev, pos);
      self.hasChanged = true;
    });

    //
    // Toolbars
    //
    scheme.find(this, 'Foreground').on('click', function() {
      self.openColorDialog('foreground');
    });
    scheme.find(this, 'Background').on('click', function() {
      self.openColorDialog('background');
    });

    var ts = Object.keys(tools);
    ts.forEach(function(t) {
      scheme.find(self, 'tool-' + t).on('click', function() {
        var stats = tools[t].statusText || '';
        statusbar.set('value', doTranslate(stats));

        self.setToolProperty('name', t);
      });
    });

    var lineWidths = [];
    for ( var i = 1; i < 22; i++ ) {
      lineWidths.push({label: i.toString(), value: i});
    }

    scheme.find(this, 'LineWidth').add(lineWidths).on('change', function(ev) {
      self.setToolProperty('lineWidth', parseInt(ev.detail, 10));
    });
    scheme.find(this, 'LineJoin').on('change', function(ev) {
      self.setToolProperty('lineJoin', ev.detail);
    });
    scheme.find(this, 'LineStroke').on('change', function(ev) {
      self.setToolProperty('lineStroke', ev.detail);
    });

    //
    // Init
    //
    this.setToolProperty('background', null);
    this.setToolProperty('foreground', null);
    this.setToolProperty('lineJoin', null);
    this.setToolProperty('lineWidth', null);
    this.setToolProperty('lineStroke', null);

    return root;
  };

  ApplicationDrawWindow.prototype.openColorDialog = function(param) {
    var self = this;

    var colorParam = null;
    if (param === 'background') {
      colorParam = doTranslate('Set background color');
    } else if (param === 'foreground') {
      colorParam = doTranslate('Set foreground color');
    }

    API.createDialog('Color', {
      title: colorParam,
      color: self.tool[param]
    }, function(ev, button, result) {
      if ( button !== 'ok' ) {
        return;
      }
      self.setToolProperty(param, result.hex);
    }, this);
  };

  ApplicationDrawWindow.prototype.setToolProperty = function(param, value) {
    console.warn('setToolProperty', param, value);

    if ( typeof this.tool[param] !== 'undefined' ) {
      if ( value !== null ) {
        this.tool[param] = value;
      }
    }

    this._scheme.find(this, 'Foreground').set('value', this.tool.foreground);
    this._scheme.find(this, 'Background').set('value', this.tool.background);
    this._scheme.find(this, 'LineJoin').set('value', this.tool.lineJoin);
    this._scheme.find(this, 'LineWidth').set('value', this.tool.lineWidth);
    this._scheme.find(this, 'LineStroke').set('value', this.tool.lineStroke);
  };

  ApplicationDrawWindow.prototype.showFile = function(file, result) {
    var self = this;
    DefaultApplicationWindow.prototype.showFile.apply(this, arguments);

    var canvas = this._scheme.find(this, 'Canvas').querySelector('canvas');
    var ctx = canvas.getContext('2d');

    function open(img) {
      if ( (window.Uint8Array && (img instanceof Uint8Array)) ) {
        var image = ctx.createImageData(canvas.width, ctx.height);
        for (var i = 0; i < img.length; i++) {
          image.data[i] = img[i];
        }
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        ctx.drawImage(image, 0, 0);
      } else if ( (img instanceof Image) || (img instanceof HTMLImageElement) ) {
        canvas.width = img.width;
        canvas.height = img.height;
        ctx.clearRect(0, 0, img.width, img.height);
        ctx.drawImage(img, 0, 0);
      }
    }

    if ( result ) {
      this._toggleLoading(true);

      var tmp = new Image();
      tmp.onerror = function() {
        self._toggleLoading(false);
        alert('Failed to open image');
      };
      tmp.onload = function() {
        self._toggleLoading(false);
        open(this);
      };
      tmp.src = result;
    } else {
      canvas.width = DEFAULT_WIDTH;
      canvas.height = DEFAULT_HEIGHT;
      ctx.clearRect(0, 0, canvas.width, canvas.height);
    }
  };

  ApplicationDrawWindow.prototype.getFileData = function() {
    var canvas = this._scheme.find(this, 'Canvas').querySelector('canvas');
    if ( canvas ) {
      return new VFS.FileDataURL(canvas.toDataURL('image/png'));
    }
    return null;
  };

  /////////////////////////////////////////////////////////////////////////////
  // APPLICATION
  /////////////////////////////////////////////////////////////////////////////

  var ApplicationDraw = function(args, metadata) {
    DefaultApplication.apply(this, ['ApplicationDraw', args, metadata, {
      readData: false,
      extension: 'png',
      mime: 'image/png',
      filename: 'New image.png',
      filetypes: [{
        label: 'PNG Image',
        mime: 'image/png',
        extension: 'png'
      }]
    }]);
  };

  ApplicationDraw.prototype = Object.create(DefaultApplication.prototype);
  ApplicationDraw.constructor = DefaultApplication;

  ApplicationDraw.prototype.init = function(settings, metadata, scheme) {
    Application.prototype.init.call(this, settings, metadata, scheme);
    var file = this._getArgument('file');
    this._addWindow(new ApplicationDrawWindow(this, metadata, scheme, file));
  };

  /////////////////////////////////////////////////////////////////////////////
  // EXPORTS
  /////////////////////////////////////////////////////////////////////////////

  OSjs.Applications = OSjs.Applications || {};
  OSjs.Applications.ApplicationDraw = OSjs.Applications.ApplicationDraw || {};
  OSjs.Applications.ApplicationDraw.Class = Object.seal(ApplicationDraw);

})(OSjs.Helpers.DefaultApplication, OSjs.Helpers.DefaultApplicationWindow, OSjs.Core.Application, OSjs.Core.Window, OSjs.Utils, OSjs.API, OSjs.VFS, OSjs.GUI);
