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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND
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

  function getSelected(view) {
    var selected = [];
    (view.get('value') || []).forEach(function(sub) {
      selected.push(sub.data);
    });
    return selected;
  }

  var notificationWasDisplayed = {};

  /////////////////////////////////////////////////////////////////////////////
  // WINDOWS
  /////////////////////////////////////////////////////////////////////////////

  function ApplicationFileManagerWindow(app, metadata, scheme, path, settings) {
    Window.apply(this, ['ApplicationFileManagerWindow', {
      icon: metadata.icon,
      title: metadata.name,
      allow_drop: true,
      width: 650,
      height: 420
    }, app, scheme]);

    this.wasFileDropped = false;
    this.currentPath = path;
    this.currentSummary = {};
    this.viewOptions = Utils.argumentDefaults(settings || {}, {
      ViewNavigation: true,
      ViewSide: true
    }, true);
    this.history = [];
    this.historyIndex = -1;

    var self = this;
    this.settingsWatch = OSjs.Core.getSettingsManager().watch('VFS', function() {
      if ( self._loaded ) {
        self.changePath();
      }
    });

    this._on('drop:upload', function(ev, item) {
      app.upload(self.currentPath, item, self);
    });

    this._on('drop:file', function(ev, src) {
      if ( Utils.dirname(src.path) !== self.currentPath ) {
        var dst = new VFS.File(Utils.pathJoin(self.currentPath, src.filename));
        self.wasFileDropped = dst;
        app.copy(src, dst, self);
      }
    });

    this._on('keydown', function(ev, keyCode, shiftKey, ctrlKey, altKey) {
      if ( Utils.keyCombination(ev, 'CTRL+V') ) {
        var clip = API.getClipboard();
        if ( clip && (clip instanceof Array) ) {
          clip.forEach(function(c) {
            if ( c && (c instanceof VFS.File) ) {
              var dst = new VFS.File(Utils.pathJoin(self.currentPath, c.filename));
              app.copy(c, dst, self);
            }
          });
        }
      } else if ( ev.keyCode === Utils.Keys.DELETE ) {
        app.rm(getSelected(self._find('FileView')), self);
      }
    });

    this._on('destroy', function() {
      try {
        OSjs.Core.getSettingsManager().unwatch(self.settingsWatch);
      } catch ( e ) {}
    });
  }

  ApplicationFileManagerWindow.prototype = Object.create(Window.prototype);
  ApplicationFileManagerWindow.constructor = Window.prototype;

  ApplicationFileManagerWindow.prototype.init = function(wm, app, scheme) {
    var root = Window.prototype.init.apply(this, arguments);
    var self = this;
    var view;

    var viewType      = this.viewOptions.ViewType || 'gui-list-view';
    var viewSide      = this.viewOptions.ViewSide === true;
    var viewNav       = this.viewOptions.ViewNavigation === true;

    var vfsOptions = Utils.cloneObject(OSjs.Core.getSettingsManager().get('VFS') || {});
    var scandirOptions = vfsOptions.scandir || {};

    var viewHidden    = scandirOptions.showHiddenFiles === true;
    var viewExtension = scandirOptions.showFileExtensions === true;

    // Load and set up scheme (GUI) here
    scheme.render(this, 'FileManagerWindow', root, null, null, {
      _: OSjs.Applications.ApplicationFileManager._
    });

    if ( (API.getConfig('Connection.Type') !== 'nw') && window.location.protocol.match(/^file/) ) { // FIXME: Translation
      this._setWarning('VFS does not work when in standalone mode');
    }

    //
    // Menus
    //

    var menuMap = {
      MenuClose: function() {
        self._close();
      },
      MenuCreateFile: function() {
        app.mkfile(self.currentPath, self);
      },
      MenuCreateDirectory:function() {
        app.mkdir(self.currentPath, self);
      },
      MenuUpload: function() {
        app.upload(self.currentPath, null, self);
      },
      MenuRename: function() {
        app.rename(getSelected(view), self);
      },
      MenuDelete: function() {
        app.rm(getSelected(view), self);
      },
      MenuInfo: function() {
        app.info(getSelected(view), self);
      },
      MenuOpen: function() {
        app.open(getSelected(view), self);
      },
      MenuDownload: function() {
        app.download(getSelected(view), self);
      },
      MenuRefresh: function() {
        self.changePath();
      },
      MenuViewList: function() {
        self.changeView('gui-list-view', true);
      },
      MenuViewTree: function() {
        self.changeView('gui-tree-view', true);
      },
      MenuViewIcon: function() {
        self.changeView('gui-icon-view', true);
      },
      MenuShowSidebar: function() {
        viewSide = self.toggleSidebar(!viewSide, true);
      },
      MenuShowNavigation: function() {
        viewNav = self.toggleNavbar(!viewNav, true);
      },
      MenuShowHidden: function() {
        viewHidden = self.toggleHidden(!viewHidden, true);
      },
      MenuShowExtension: function() {
        viewExtension = self.toggleExtension(!viewExtension, true);
      },
      MenuColumnFilename: function() {
        self.toggleColumn('filename', true);
      },
      MenuColumnMIME: function() {
        self.toggleColumn('mime', true);
      },
      MenuColumnCreated: function() {
        self.toggleColumn('ctime', true);
      },
      MenuColumnModified: function() {
        self.toggleColumn('mtime', true);
      },
      MenuColumnSize: function() {
        self.toggleColumn('size', true);
      }
    };

    function menuEvent(ev) {
      var f = ev.detail.func || ev.detail.id;
      if ( menuMap[f] ) {
        menuMap[f]();
      }
    }

    scheme.find(this, 'SubmenuFile').on('select', menuEvent);
    var contextMenu = scheme.find(this, 'SubmenuContext').on('select', menuEvent);
    scheme.find(this, 'SubmenuEdit').on('select', menuEvent);
    var viewMenu = scheme.find(this, 'SubmenuView').on('select', menuEvent);

    viewMenu.set('checked', 'MenuViewList', viewType === 'gui-list-view');
    viewMenu.set('checked', 'MenuViewTree', viewType === 'gui-tree-view');
    viewMenu.set('checked', 'MenuViewIcon', viewType === 'gui-icon-view');
    viewMenu.set('checked', 'MenuShowSidebar', viewSide);
    viewMenu.set('checked', 'MenuShowNavigation', viewNav);
    viewMenu.set('checked', 'MenuShowHidden', viewHidden);
    viewMenu.set('checked', 'MenuShowExtension', viewExtension);

    //
    // Toolbar
    //
    scheme.find(this, 'GoLocation').on('enter', function(ev) {
      self.changePath(ev.detail, null, false, true);
    });
    scheme.find(this, 'GoBack').on('click', function(ev) {
      self.changeHistory(-1);
    });
    scheme.find(this, 'GoNext').on('click', function(ev) {
      self.changeHistory(1);
    });

    //
    // Side View
    //
    var side = scheme.find(this, 'SideView');
    side.on('activate', function(ev) {
      if ( ev && ev.detail && ev.detail.entries ) {
        var entry = ev.detail.entries[0];
        if ( entry ) {
          self.changePath(entry.data.root);
        }
      }
    });

    //
    // File View
    //
    view = this._scheme.find(this, 'FileView');
    view.on('activate', function(ev) {
      if ( ev && ev.detail && ev.detail.entries ) {
        self.checkActivation(ev.detail.entries);
      }
    });
    view.on('select', function(ev) {
      if ( ev && ev.detail && ev.detail.entries ) {
        self.checkSelection(ev.detail.entries);
      }
    });
    view.on('contextmenu', function(ev) {
      if ( ev && ev.detail && ev.detail.entries ) {
        self.checkSelection(ev.detail.entries);
      }
      contextMenu.show(ev);
    });

    //
    // Init
    //

    this.renderSideView();

    this.changeView(viewType, false);
    this.toggleHidden(viewHidden, false);
    this.toggleExtension(viewExtension, false);
    this.toggleSidebar(viewSide, false);
    this.toggleNavbar(viewNav, false);

    this.changePath(this.currentPath);
    this.toggleColumn();

    return root;
  };

  ApplicationFileManagerWindow.prototype.checkSelection = function(files) {
    var scheme = this._scheme;

    if ( !scheme ) {
      return;
    }

    var self = this;
    var content = '';
    var statusbar = scheme.find(this, 'Statusbar');
    var doTranslate = OSjs.Applications.ApplicationFileManager._;

    var sum, label;

    function toggleMenuItems(isFile, isDirectory) {
      /*
       * Toggling MenuItems with the bit MODE_F or MODE_FD set by type of selected items
       * MODE_F : Selected items consist of ONLY files
       * MODE_FD : One or many items are selected (type doesn't matter)
       */

      var MODE_F = !isFile || !!isDirectory;
      var MODE_FD = !(isFile || isDirectory);

      scheme.find(self, 'MenuRename').set('disabled', MODE_FD);
      scheme.find(self, 'MenuDelete').set('disabled', MODE_FD);
      scheme.find(self, 'MenuInfo').set('disabled', MODE_FD);  // TODO: Directory info must be supported
      scheme.find(self, 'MenuDownload').set('disabled', MODE_F);
      scheme.find(self, 'MenuOpen').set('disabled', MODE_F);

      scheme.find(self, 'ContextMenuRename').set('disabled', MODE_FD);
      scheme.find(self, 'ContextMenuDelete').set('disabled', MODE_FD);
      scheme.find(self, 'ContextMenuInfo').set('disabled', MODE_FD);  // TODO: Directory info must be supported
      scheme.find(self, 'ContextMenuDownload').set('disabled', MODE_F);
      scheme.find(self, 'ContextMenuOpen').set('disabled', MODE_F);
    }

    if ( files && files.length ) {
      sum = {files: 0, directories: 0, size: 0};
      (files || []).forEach(function(f) {
        if ( f.data.type === 'dir' ) {
          sum.directories++;
        } else {
          sum.files++;
          sum.size += f.data.size;
        }
      });

      label = 'Selected {0} files, {1} dirs, {2}';
      content = doTranslate(label, sum.files, sum.directories, Utils.humanFileSize(sum.size));

      toggleMenuItems(sum.files, sum.directories);
    } else {
      sum = this.currentSummary;
      if ( sum ) {
        label = 'Showing {0} files ({1} hidden), {2} dirs, {3}';
        content = doTranslate(label, sum.files, sum.hidden, sum.directories, Utils.humanFileSize(sum.size));
      }

      toggleMenuItems(false, false);
    }

    statusbar.set('value', content);
  };

  ApplicationFileManagerWindow.prototype.checkActivation = function(files) {
    var self = this;
    (files || []).forEach(function(f) {
      if ( f.data.type === 'dir' ) {
        self.changePath(f.data.path);
        return false;
      }

      API.open(new VFS.File(f.data));

      return true;
    });
  };

  ApplicationFileManagerWindow.prototype.updateSideView = function(updateModule) {
    if ( this._destroyed || !this._scheme ) {
      return;
    }

    var found = null;
    var path = this.currentPath || '/';

    if ( updateModule ) {
      this.renderSideView();
    }

    OSjs.Core.getMountManager().getModules({special: true}).forEach(function(m, i) {
      if ( path.match(m.module.match) ) {
        found = m.module.root;
      }
    });

    var view = this._scheme.find(this, 'SideView');
    view.set('selected', found, 'root');
  };

  ApplicationFileManagerWindow.prototype.renderSideView = function() {
    if ( this._destroyed || !this._scheme ) {
      return;
    }

    var sideViewItems = [];
    OSjs.Core.getMountManager().getModules({special: true}).forEach(function(m, i) {
      if ( m.module.dynamic && !m.module.mounted() ) {
        return;
      }

      var classNames = [m.module.mounted() ? 'mounted' : 'unmounted'];
      if ( m.module.readOnly ) {
        classNames.push('readonly gui-has-emblem');
      }

      sideViewItems.push({
        value: m.module,
        className: classNames.join(' '),
        columns: [
          {
            label: m.module.description,
            icon: API.getIcon(m.module.icon)
          }
        ],
        onCreated: function(nel) {
          if ( m.module.readOnly ) {
            nel.style.backgroundImage = 'url(' + API.getIcon('emblems/emblem-readonly.png', '16x16') + ')';
          }
        }
      });
    });

    var side = this._scheme.find(this, 'SideView');
    side.clear();
    side.add(sideViewItems);
  };

  ApplicationFileManagerWindow.prototype.onMountEvent = function(module, msg) {
    var m = OSjs.Core.getMountManager().getModule(module);
    if ( m ) {
      if ( msg === 'vfs:unmount' ) {
        if ( this.currentPath.match(m.match) ) {
          this.changePath(API.getDefaultPath());
        }
      }
      this.updateSideView(m);
    }
  };

  ApplicationFileManagerWindow.prototype.onFileEvent = function(chk, isDest) {
    if ( (this.currentPath === Utils.dirname(chk.path)) || (this.currentPath === chk.path) ) {
      this.changePath(null, this.wasFileDropped, false, false, !this.wasFileDroped);
    }
  };

  ApplicationFileManagerWindow.prototype.changeHistory = function(dir) {
    if ( this.historyIndex !== -1 ) {
      if ( dir < 0 ) {
        if ( this.historyIndex > 0 ) {
          this.historyIndex--;
        }
      } else if ( dir > 0 ) {
        if ( this.historyIndex < this.history.length - 1 ) {
          this.historyIndex++;
        }
      }

      this.changePath(this.history[this.historyIndex], null, true);
    }
  };

  ApplicationFileManagerWindow.prototype.changePath = function(dir, selectFile, isNav, isInput, applyScroll) {
    if ( this._destroyed || !this._scheme ) {
      return;
    }
    this.wasFileDropped = false;

    //if ( dir === this.currentPath ) { return; }
    dir = dir || this.currentPath;

    var self = this;
    var view = this._scheme.find(this, 'FileView');

    function updateNavigation() {
      self._scheme.find(self, 'GoLocation').set('value', dir);
      self._scheme.find(self, 'GoBack').set('disabled', self.historyIndex <= 0);
      self._scheme.find(self, 'GoNext').set('disabled', self.historyIndex < 0 || self.historyIndex >= (self.history.length - 1));
    }

    function updateHistory(dir) {
      if ( !isNav ) {
        if ( self.historyIndex >= 0 && self.historyIndex < self.history.length - 1 ) {
          self.history = [];
        }

        var current = self.history[self.history.length - 1];
        if ( current !== dir ) {
          self.history.push(dir);
        }

        if ( self.history.length > 1 ) {
          self.historyIndex = self.history.length - 1;
        } else {
          self.historyIndex = -1;
        }
      }
      if ( isInput ) {
        self.history = [dir];
        self.historyIndex = 0;
      }

      self._setTitle(dir, true);
    }

    this._toggleLoading(true);

    view._call('chdir', {
      path: dir,
      done: function(error, summary) {
        if ( self._destroyed || !self._scheme ) {
          return;
        }

        if ( dir && !error ) {
          self.currentPath = dir;
          self.currentSummary = summary;
          if ( self._app ) {
            self._app._setArgument('path', dir);
          }
          updateHistory(dir);
        }
        self._toggleLoading(false);

        self.checkSelection([]);
        self.updateSideView();

        if ( selectFile && view ) {
          view.set('selected', selectFile.filename, 'filename', {
            scroll: applyScroll
          });
        }

        updateNavigation();
      }
    });

  };

  ApplicationFileManagerWindow.prototype.changeView = function(viewType, set) {
    if ( this._destroyed || !this._scheme ) {
      return;
    }

    var view = this._scheme.find(this, 'FileView');
    view.set('type', viewType, !!set);

    if ( set ) {
      this._app._setSetting('ViewType', viewType, true);
    }
  };

  ApplicationFileManagerWindow.prototype.toggleSidebar = function(toggle, set) {
    if ( this._destroyed || !this._scheme ) {
      return;
    }

    this.viewOptions.ViewSide = toggle;

    var container = this._scheme.find(this, 'SideContainer');
    var handle = new GUI.Element(container.$element.parentNode.querySelector('gui-paned-view-handle'));
    if ( toggle ) {
      container.show();
      handle.show();
    } else {
      container.hide();
      handle.hide();
    }
    if ( set ) {
      this._app._setSetting('ViewSide', toggle, true);
    }
    return toggle;
  };

  ApplicationFileManagerWindow.prototype.toggleVFSOption = function(opt, key, toggle, set) {
    if ( this._destroyed || !this._scheme ) {
      return;
    }

    var view = this._scheme.find(this, 'FileView');
    var vfsOptions = OSjs.Core.getSettingsManager().instance('VFS');

    var opts = {scandir: {}};
    opts.scandir[opt] = toggle;

    vfsOptions.set(null, opts, null, set); // set triggers refresh because of watch
    view.set(key, toggle);

    return toggle;
  };

  ApplicationFileManagerWindow.prototype.toggleHidden = function(toggle, set) {
    if ( this._destroyed || !this._scheme ) {
      return;
    }

    return this.toggleVFSOption('showHiddenFiles', 'dotfiles', toggle, set);
  };

  ApplicationFileManagerWindow.prototype.toggleExtension = function(toggle, set) {
    if ( this._destroyed || !this._scheme ) {
      return;
    }

    return this.toggleVFSOption('showFileExtensions', 'extensions', toggle, set);
  };

  ApplicationFileManagerWindow.prototype.toggleNavbar = function(toggle, set) {
    if ( this._destroyed || !this._scheme ) {
      return;
    }

    this.viewOptions.ViewNavigation = toggle;

    var viewNav  = this._scheme.find(this, 'ToolbarContainer');
    if ( toggle ) {
      viewNav.show();
    } else {
      viewNav.hide();
    }

    if ( set ) {
      this._app._setSetting('ViewNavigation', toggle, true);
    }

    return toggle;
  };

  ApplicationFileManagerWindow.prototype.toggleColumn = function(col, set) {
    if ( this._destroyed || !this._scheme ) {
      return;
    }

    var vfsOptions     = Utils.cloneObject(OSjs.Core.getSettingsManager().get('VFS') || {});
    var scandirOptions = vfsOptions.scandir || {};
    var viewColumns    = scandirOptions.columns || ['filename', 'mime', 'size'];

    if ( col ) {
      var found = viewColumns.indexOf(col);
      if ( found >= 0 ) {
        viewColumns.splice(found, 1);
      } else {
        viewColumns.push(col);
      }

      scandirOptions.columns = viewColumns;

      OSjs.Core.getSettingsManager().set('VFS', 'scandir', scandirOptions, set);
    }

    var viewMenu = this._scheme.find(this, 'SubmenuView');
    viewMenu.set('checked', 'MenuColumnFilename', viewColumns.indexOf('filename') >= 0);
    viewMenu.set('checked', 'MenuColumnMIME', viewColumns.indexOf('mime') >= 0);
    viewMenu.set('checked', 'MenuColumnCreated', viewColumns.indexOf('ctime') >= 0);
    viewMenu.set('checked', 'MenuColumnModified', viewColumns.indexOf('mtime') >= 0);
    viewMenu.set('checked', 'MenuColumnSize', viewColumns.indexOf('size') >= 0);
  };

  /////////////////////////////////////////////////////////////////////////////
  // APPLICATION
  /////////////////////////////////////////////////////////////////////////////

  var ApplicationFileManager = function(args, metadata) {
    Application.apply(this, ['ApplicationFileManager', args, metadata]);
  };

  ApplicationFileManager.prototype = Object.create(Application.prototype);
  ApplicationFileManager.constructor = Application;

  ApplicationFileManager.prototype.destroy = function() {
    return Application.prototype.destroy.apply(this, arguments);
  };

  ApplicationFileManager.prototype.init = function(settings, metadata, scheme) {
    Application.prototype.init.apply(this, arguments);

    var self = this;
    var path = this._getArgument('path') || API.getDefaultPath();

    this._on('vfs', function(msg, obj) {
      var win = self._getMainWindow();
      if ( win ) {
        if ( msg === 'vfs:mount' || msg === 'vfs:unmount' ) {
          win.onMountEvent(obj, msg);
        } else {
          if ( obj.destination ) {
            win.onFileEvent(obj.destination, true);
            win.onFileEvent(obj.source);
          } else {
            win.onFileEvent(obj);
          }
        }
      }
    });

    this._addWindow(new ApplicationFileManagerWindow(this, metadata, scheme, path, settings));
  };

  ApplicationFileManager.prototype.download = function(items) {
    items.forEach(function(item) {
      VFS.url(new VFS.File(item), function(error, result) {
        if ( result ) {
          window.open(result);
        }
      });
    });
  };

  ApplicationFileManager.prototype.rm = function(items, win) {
    var self = this;

    // TODO: These must be async
    var files = [];
    items.forEach(function(i) {
      files.push(i.filename);
    });
    files = files.join(', ');

    win._toggleDisabled(true);
    API.createDialog('Confirm', {
      buttons: ['yes', 'no'],
      message: Utils.format(OSjs.Applications.ApplicationFileManager._('Delete **{0}** ?'), files)
    }, function(ev, button) {
      win._toggleDisabled(false);
      if ( button !== 'ok' && button !== 'yes' ) {
        return;
      }

      items.forEach(function(item) {
        item = new VFS.File(item);
        self._action('delete', [item], function() {
          win.changePath(null);
        });
      });
    }, win);

  };

  ApplicationFileManager.prototype.info = function(items, win) {
    items.forEach(function(item) {
      if ( item.type === 'file' ) {
        API.createDialog('FileInfo', {
          file: new VFS.File(item)
        }, null, win);
      }
    });
  };

  ApplicationFileManager.prototype.open = function(items) {
    items.forEach(function(item) {
      if ( item.type === 'file' ) {
        API.open(new VFS.File(item), {forceList: true});
      }
    });
  };

  ApplicationFileManager.prototype.rename = function(items, win) {
    // TODO: These must be async
    var self = this;

    function rename(item, newName) {
      item = new VFS.File(item);

      var newitem = new VFS.File(item);
      newitem.filename = newName;
      newitem.path = Utils.replaceFilename(item.path, newName);

      self._action('move', [item, newitem], function(error) {
        if ( !error ) {
          win.changePath(null, newitem);
        }
      });
    }

    items.forEach(function(item) {
      var dialog = API.createDialog('Input', {
        message: OSjs.Applications.ApplicationFileManager._('Rename **{0}**', item.filename),
        value: item.filename
      }, function(ev, button, result) {
        if ( button === 'ok' && result ) {
          rename(item, result);
        }
      }, win);
      dialog.setRange(Utils.getFilenameRange(item.filename));
    });
  };

  ApplicationFileManager.prototype.mkfile = function(dir, win) {
    var self = this;

    win._toggleDisabled(true);
    function finished(write, item) {
      win._toggleDisabled(false);

      if ( item ) {
        VFS.write(item, '', function() {
          win.changePath(null, item);
        }, {}, self);
      }
    }

    API.createDialog('Input', {
      value: 'My new File',
      message: OSjs.Applications.ApplicationFileManager._('Create a new file in **{0}**', dir)
    }, function(ev, button, result) {
      if ( !result ) {
        win._toggleDisabled(false);
        return;
      }

      var item = new VFS.File(dir + '/' + result);
      VFS.exists(item, function(error, result) {
        if ( result ) {
          win._toggleDisabled(true);

          API.createDialog('Confirm', {
            buttons: ['yes', 'no'],
            message: API._('DIALOG_FILE_OVERWRITE', item.filename)
          }, function(ev, button) {
            finished(button === 'yes' || button === 'ok', item);
          }, self);
        } else {
          finished(true, item);
        }
      });
    }, win);
  };

  ApplicationFileManager.prototype.mkdir = function(dir, win) {
    var self = this;

    win._toggleDisabled(true);
    API.createDialog('Input', {
      message: OSjs.Applications.ApplicationFileManager._('Create a new directory in **{0}**', dir)
    }, function(ev, button, result) {
      if ( !result ) {
        win._toggleDisabled(false);
        return;
      }

      var item = new VFS.File(dir + '/' + result);
      self._action('mkdir', [item], function() {
        win._toggleDisabled(false);
        win.changePath(null, item);
      });
    }, win);
  };

  ApplicationFileManager.prototype.copy = function(src, dest, win) {
    var self = this;
    var dialog = API.createDialog('FileProgress', {
      message: OSjs.Applications.ApplicationFileManager._('Copying **{0}** to **{1}**', src.filename, dest.path)
    }, function() {
    }, win);

    win._toggleLoading(true);

    VFS.copy(src, dest, function(error, result) {
      win._toggleLoading(false);

      try {
        dialog._close();
      } catch ( e ) {}

      if ( error ) {
        API.error(API._('ERR_GENERIC_APP_FMT', self.__label), API._('ERR_GENERIC_APP_REQUEST'), error);
        return;
      }
    }, {dialog: dialog}, this._app);
  };

  ApplicationFileManager.prototype.upload = function(dest, files, win) {
    var self = this;

    function upload() {
      win._toggleLoading(true);

      VFS.upload({
        files: files,
        destination: dest,
        win: win,
        app: self
      }, function(error, file) {
        win._toggleLoading(false);
        if ( error ) {
          API.error(API._('ERR_GENERIC_APP_FMT', self.__label), API._('ERR_GENERIC_APP_REQUEST'), error);
          return;
        }
        win.changePath(null, file, false, false, true);
      });
    }

    if ( files ) {
      upload();
    } else {
      API.createDialog('FileUpload', {
        dest: dest
      }, function(ev, button, result) {
        if ( result ) {
          win.changePath(null, result);
        }
      }, win);
    }
  };

  ApplicationFileManager.prototype.showStorageNotification = function(type) {
    if ( notificationWasDisplayed[type] ) {
      return;
    }
    notificationWasDisplayed[type] = true;

    var wm = OSjs.Core.getWindowManager();
    if ( wm ) {
      wm.notification({
        title: 'External Storage',
        message: 'Using external services requires authorization. A popup-window may appear.',
        icon: 'status/dialog-information.png'
      });
    }
  };

  ApplicationFileManager.prototype._action = function(name, args, callback) {
    callback = callback || function() {};
    var self = this;
    var _onError = function(error) {
      API.error(API._('ERR_GENERIC_APP_FMT', self.__label), API._('ERR_GENERIC_APP_REQUEST'), error);
      callback(false);
    };

    VFS[name].apply(VFS, args.concat(function(error, result) {
      if ( error ) {
        _onError(error);
        return;
      }
      callback(error, result);
    }, null, this));
  };

  /////////////////////////////////////////////////////////////////////////////
  // EXPORTS
  /////////////////////////////////////////////////////////////////////////////

  OSjs.Applications = OSjs.Applications || {};
  OSjs.Applications.ApplicationFileManager = OSjs.Applications.ApplicationFileManager || {};
  OSjs.Applications.ApplicationFileManager.Class = Object.seal(ApplicationFileManager);

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
(function(Application, Window, GUI, Utils, API, VFS) {
  // jscs:disable validateQuoteMarks
  'use strict';

  /////////////////////////////////////////////////////////////////////////////
  // LOCALES
  /////////////////////////////////////////////////////////////////////////////

  var _Locales = {
    bg_BG : {
      'Copying file...' : 'Копиране на файл...',
      "Copying **{0}** to **{1}**" : "Копиране **{0}** към **{1}**",
      "Refreshing..." : "Опресняване...",
      "Loading..." : "Зареждане...",
      "Create a new directory in **{0}**" : "Създаване на нова директория в **{0}**",
      "Rename **{0}**" : "преименуване на **{0}**",
      "Delete **{0}** ?" : "Изтриване на **{0}**?"
    },
    de_DE : {
      'Copying file...' : 'Kopiere Datei...',
      "Copying **{0}** to **{1}**" : "Kopiere **{0}** nach **{1}**",
      "Refreshing..." : "Aktualisiere...",
      "Loading..." : "Lade...",
      "Create a new directory in **{0}**" : "Erstelle ein neues Verzeichnis in **{0}**",
      "Rename **{0}**" : "**{0}** umbenennen",
      "Delete **{0}** ?" : "**{0}** löschen?"
    },
    fr_FR : {
      'Copying file...' : 'Copie de fichier...',
      "Copying **{0}** to **{1}**" : "Copie de **{0}** à **{1}**",
      "Refreshing..." : "Rafraichissement...",
      "Loading..." : "Chargement...",
      "Create a new file in **{0}**" : "Créer un nouveau fichier dans **{0}**",
      "Create a new directory in **{0}**" : "Créer un nouveau dossier dans **{0}**",
      "Rename **{0}**" : "Renommer **{0}**",
      "Delete **{0}** ?" : "Supprimer **{0}** ?",
      'Selected {0} files, {1} dirs, {2}' : '{0} fichier(s) selectionné(s), {1} dossier(s), {2}',
      'Showing {0} files ({1} hidden), {2} dirs, {3}' : '{0} fichier(s) affiché(s) ({1} caché(s)), {2} dossier(s), {3}'
    },
    ar_DZ : {
      'Copying file...' : 'جاري نسخ الملف...',
      "Copying **{0}** to **{1}**" : "نسخ من **{0}** إلى **{1}**",
      "Refreshing..." : "جاري التحديث...",
      "Loading..." : "جاري التحميل...",
      "Create a new file in **{0}**" : "إنشاء ملف جديد في **{0}**",
      "Create a new directory in **{0}**" : "إنشاء مجلد جديد في **{0}**",
      "Rename **{0}**" : "إعادة التسمية **{0}**",
      "Delete **{0}** ?" : "حذف **{0}** ?",
      'Selected {0} files, {1} dirs, {2}' : '{0} ملف مختار, {1} مجلد, {2}',
      'Showing {0} files ({1} hidden), {2} dirs, {3}' : '{0} ملف مرئي ({1} مخفي(s)), {2} مجلد, {3}'
    },
    it_IT : {
      'Copying file...' : 'Copiamento file...',
      "Copying **{0}** to **{1}**" : "Copia **{0}** in **{1}**",
      "Refreshing..." : "Ricarica...",
      "Loading..." : "Caricamento...",
      "Create a new file in **{0}**" : "Creazione nuovo file in **{0}**",
      "Create a new directory in **{0}**" : "Creazione nuova cartella in **{0}**",
      "Rename **{0}**" : "Rinomina **{0}**",
      "Delete **{0}** ?" : "Cancellare **{0}** ?",
      'Selected {0} files, {1} dirs, {2}' : '{0} file selezionati, {1} cartelle, {2}',
      'Showing {0} files ({1} hidden), {2} dirs, {3}' : 'Mostrando {0} file(s) ({1} nascosti), {2} cartelle, {3}'
    },
    ko_KR : {
      'Copying file...' : '파일 복사...',
      "Copying **{0}** to **{1}**" : "**{0}**를 **{1}**으로 복사",
      "Refreshing..." : "새로고치는 중...",
      "Loading..." : "기다려주세요...",
      "Create a new file in **{0}**" : "**{0}**에 새 파일 만들기",
      "Create a new directory in **{0}**" : "**{0}**에 새 디렉토리 만들기",
      "Rename **{0}**" : "**{0}**의 이름 바꾸기",
      "Delete **{0}** ?" : "**{0}**을 삭제하시겠습니까?",
      'Selected {0} files, {1} dirs, {2}' : '{0} 개의 파일, {1} 개의 디렉토리가 선택됨, {2}',
      'Showing {0} files ({1} hidden), {2} dirs, {3}' : '{0} 개의 파일({1} 개의 숨긴 파일), {2} 개의 디렉토리가 존재, {3}'
    },
    nl_NL : {
      'Copying file...' : 'Bestand kopieren...',
      "Copying **{0}** to **{1}**" : "Kopieer **{0}** naar **{1}**",
      "Refreshing..." : "Vernieuwen...",
      "Loading..." : "Laden...",
      "Create a new directory in **{0}**" : "Maak een nieuwe map in **{0}**",
      "Rename **{0}**" : "Hernoem **{0}**",
      "Delete **{0}** ?" : "**{0}** verwijderen?"
    },
    no_NO : {
      'Copying file...' : 'Kopierer fil...',
      "Copying **{0}** to **{1}**" : "Kopierer **{0}** to **{1}**",
      "Refreshing..." : "Gjenoppfrisker...",
      "Loading..." : "Laster...",
      "Create a new file in **{0}**" : "Opprett ny fil i **{0}**",
      "Create a new directory in **{0}**" : "Opprett ny mappe i **{0}**",
      "Rename **{0}**" : "Navngi **{0}**",
      "Delete **{0}** ?" : "Slette **{0}** ?"
    },
    pl_PL : {
      'Copying file...' : 'Kopiowanie pliku...',
      "Copying **{0}** to **{1}**" : "Kopiowanie **{0}** do **{1}**",
      "Refreshing..." : "Odświeżanie...",
      "Loading..." : "Ładowanie...",
      "Create a new file in **{0}**" : "Utwórz nowy plik w **{0}**",
      "Create a new directory in **{0}**" : "Utwórz nowy folder w **{0}**",
      "Rename **{0}**" : "Zmień nazwę **{0}**",
      "Delete **{0}** ?" : "Usunąć **{0}** ?",
      'Selected {0} files, {1} dirs, {2}' : 'Wybrane pliki: {0}, foldery: {1}, {2}',
      'Showing {0} files ({1} hidden), {2} dirs, {3}' : 'Pokazywane pliki: {0} /(ukryte: {1}, foldery: {2}, {3}'
    },
    ru_RU : {
      'Copying file...' : 'Копирование файла...',
      "Copying **{0}** to **{1}**" : "Копирование **{0}** в **{1}**",
      "Refreshing..." : "Обновление...",
      "Loading..." : "Загрузка...",
      "Create a new directory in **{0}**" : "Создать новый каталог в **{0}**",
      "Rename **{0}**" : "Переименовать **{0}**",
      "Delete **{0}** ?" : "Удалить **{0}** ?"
    },
    sk_SK : {
      'Copying file...' : 'Kopírujem súbor...',
      "Copying **{0}** to **{1}**" : "Kopírujem **{0}** do **{1}**",
      "Refreshing..." : "Obnovujem...",
      "Loading..." : "Nahrávam...",
      "Create a new file in **{0}**" : "Vytvor nový súbor v **{0}**",
      "Create a new directory in **{0}**" : "Vytvor nový adresár v **{0}**",
      "Rename **{0}**" : "Premenuj **{0}**",
      "Delete **{0}** ?" : "Zmazať **{0}** ?"
    },
    tr_TR : {
      'Copying file...' : 'kopyalanıyor...',
      "Copying **{0}** to **{1}**" : "**{0}** dosyası  **{1}**e kopyalanıyor",
      "Refreshing..." : "yenileniyor...",
      "Loading..." : "yükleniyor...",
      "Create a new directory in **{0}**" : " **{0}** içinde yeni bir klasör aç",
      "Rename **{0}**" : "yeniden adlandır **{0}**",
      "Delete **{0}** ?" : "sil **{0}**?"
    },
    vi_VN : {
      'Copying file...' : 'Đang sao chép...',
      "Copying **{0}** to **{1}**" : "Đang chép **{0}** tới **{1}**",
      "Refreshing..." : "Đang làm mới...",
      "Loading..." : "Đang tải...",
      "Create a new file in **{0}**" : "Tạo một tập tin mới trong **{0}**",
      "Create a new directory in **{0}**" : "Tạo một thư mục mới trong **{0}**",
      "Rename **{0}**" : "Đổi tên **{0}**",
      "Delete **{0}** ?" : "Xóa **{0}**?",
      'Selected {0} files, {1} dirs, {2}' : 'Đã chọn {0} tập tin, {1} thư mục, {2}',
      'Showing {0} files ({1} hidden), {2} dirs, {3}' : 'Đang hiển thị {0} tập tin({1} bị ẩn), {2} thư mục, {3}'
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
  OSjs.Applications.ApplicationFileManager = OSjs.Applications.ApplicationFileManager || {};
  OSjs.Applications.ApplicationFileManager._ = _;

})(OSjs.Core.Application, OSjs.Core.Window, OSjs.GUI, OSjs.Utils, OSjs.API, OSjs.VFS);
