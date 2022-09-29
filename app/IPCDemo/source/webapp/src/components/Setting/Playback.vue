<template>
  <el-container class="playback_container">
    <el-main class="playback_view_left_container">
      <el-container class="preview_container">
        <el-main ref="axMain" class="video_container">
          <video id="myVideo" class="axVideo" controls="control" muted playsinline oncontextmenu="return false;">
          </video>
        </el-main>
      </el-container>
    </el-main>
    <el-aside class="panel_container" width="350px">
      <div class="panel_view">
        <p class="notice" v-show="play_list.length == 0">未发现录像文件</p>
        <div class="">
          <input id="chkAll" type="checkbox" style="margin-left: 10px" @change="selectAll()">全选&nbsp;
          <el-button type="primary" size="small" @click="removeSelected()">删除</el-button>
          <el-button type="primary" size="small" @click="downloadSelected()">下载</el-button>
        </div>
        <div :for="index" v-for="(item, index) in play_list" :key="index" :class="{'record_normal':!item.focus, 'record_play':item.focus}">
          <input type="checkbox" :value="item.name" v-model="item.select"><a @click="play(index)">{{item.name}}</a>&nbsp;
          <el-button type="info" size="mini" class="download_btn" @click="download(index)">下载</el-button>
        </div>
      </div>
    </el-aside>
  </el-container>
</template>

<script>
export default {
  data () {
    return {
      rec_threshold: 16 * 1024*1024*1024,
      is_loaded: false,
      is_loading: false,
      play_list: [],
      cur_index: 0,
      _fs: null
    }
  },
  created () {
    console.log('Playback++');
    this.entry_record_files();
  },
  methods: {
    errorHandler(err) {
      var msg = 'An error occured(errcd): ';
      console.log(msg + err.code);
    },
    entry_record_files() {
      var _self = this;
      window.webkitRequestFileSystem(window.PERSISTENT, this.rec_threshold,
        function (fs) {
          _self._fs = fs;
          fs.root.getDirectory('ax_video', {create: true}, function(dirEntry) {
            var dirReader = dirEntry.createReader();
            dirReader.readEntries(function(entries) {
              // clear play list
              _self.play_list = [];
              if (entries.length > 0) {
                // entry & add record file(s) to play list
                for(var i = 0; i < entries.length; i++) {
                  var entry = entries[i];
                  if (entry.isFile){
                    fs.root.getFile('ax_video/' + entry.name, {}, function(fileEntry) {
                      fileEntry.createWriter(function(fileWriter) {
                        if (fileWriter.length > 100 * 1024) {
                          var _focus = false;
                          if (!_self.is_loaded) {
                            // load first record by default
                            _self.load_record_file(fileEntry.name);
                            _self.is_loaded = true;
                            _focus = true;
                          }
                          // add to play list
                          _self.play_list.push({name: fileEntry.name, select: false, focus: _focus});
                        }
                        else {
                          fileEntry.remove(function() {
                            console.log(fileEntry.toURL() + ' is removed');
                          });
                        }
                      }, _self.errorHandler);
                    }, _self.errorHandler);
                  }
                }
              }
            }, _self.errorHandler);
          }, _self.errorHandler);
        });
    },
    load_record_file(fileNm, bPlay=false) {
      if (null == this._fs)
        return;

      var _self = this;
      const myVideo = document.getElementById('myVideo');
      myVideo.pause();
      myVideo.currentTime = 0;
      myVideo.src = null;

      this._fs.root.getFile('ax_video/' + fileNm, {}, function(fileEntry) {
        fileEntry.file(function (file) {
          myVideo.src = window.URL.createObjectURL(file);
          if (bPlay) {
            myVideo.play();
            _self.is_loading = false;
          }
        });
      }, this.errorHandler);
    },
    play(index) {
      if (this.is_loading || index == this.cur_index) {
        return;
      }

      if (index < this.play_list.length) {
        for (var i = 0;i < this.play_list.length;i++) {
          if (index != i) {
            this.play_list[i].focus = false;
          }
          else {
            this.play_list[i].focus = true;
          }
        }

        this.is_loading = true;
        this.cur_index = index;
        this.load_record_file(this.play_list[index].name, true);
      }
    },
    download(index) {
      var _self = this;
      if (index < this.play_list.length) {
        this._fs.root.getFile('ax_video/' + this.play_list[index].name, {}, function(fileEntry) {
          fileEntry.file(function (file) {
            var _url = window.URL.createObjectURL(file);
            var _a = document.createElement('a');
            _a.style.display = 'none';
            _a.href = _url;
            _a.download = _self.play_list[index].name;
            _a.dispatchEvent(new MouseEvent('click'));
          });
        }, this.errorHandler);
      }
    },
    removeSelected() {
      var _sel = [];
      for (var _dict of this.play_list) {
          if (_dict.select) {
            _sel.push(_dict.name);
          }
      }
      if (0 == _sel.length)
        return;

      if (confirm('确定要删除文件吗？') == true) {
        var _self = this;
        this.is_loaded = false;
        this.is_loading = false;
        this.cur_index = 0;

        const myVideo = document.getElementById('myVideo');
        myVideo.pause();
        myVideo.currentTime = 0;
        myVideo.src = null;

        for (var _file of _sel) {
          this._fs.root.getFile('ax_video/' + _file, {}, function(fileEntry) {
            fileEntry.remove(function() {
              console.log(fileEntry.toURL() + ' is removed');
            });
          }, _self.errorHandler);
        }

        this.entry_record_files();
      }
    },
    downloadSelected() {
      var _self = this;
      var index = 0;
      for (var i = 0;i < this.play_list.length;i++) {
        if (this.play_list[i].select) {
          let _i = i;
          setTimeout(()=>{
            _self.download(_i);
          }, (++index) * 300);
        }
      }
    },
    selectAll() {
      const chkAll = document.getElementById('chkAll');
      for (var _dict of this.play_list) {
        _dict.select = chkAll.checked ? true : false;
      }
    }
  }
}
</script>

<style lang="less" scoped>
.playback_container {
  height: 100%!important;
}
.playback_view_left_container {
  height: 100%!important;
  padding: 0!important;
}
.preview_container{
  height: 100%!important;
  width: 100%;
  .video_container {
    padding: 0!important;
    height: 100%;
    width: 100%;
    background-color: #000;
    display: flex;
    flex-direction: column;
    align-content: center;
    align-items: center;
    .axVideo {
      height: 100%;
      width: 100%;
      object-fit: fill!important;
      display: inline-block;
    }
  }
}
.panel_container {
  height: 100%;
  margin-left: 10px;
  background-color: #fff;
  .panel_view {
    margin-top: 20px;
  }
}
.notice {
  color: #000;
  margin-left:20px;
  font-size: 18px;
  font-weight: bold;
}
.download_btn {
  margin-left:15px;
}
.record_normal {
  background-color: #fff;
  margin-left: 5px;
  margin-top: 3px;
  margin-bottom: 3px;
  cursor: pointer;
}
.record_play {
  background-color: rgb(240, 188, 77);
  margin-left: 5px;
  margin-top: 3px;
  margin-bottom: 3px;
  cursor: pointer;
}
.el-input-number {
  width: 120px;
}
.el-select {
  width: 100px;
}
.inline {
  display: inline-block;
}
</style>
