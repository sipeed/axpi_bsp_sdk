<template>
  <div class="setting">
    <el-container class="seting-container">
      <el-aside width="150px">
        <div class="left-container">
          <el-menu router :default-active="activePath" class="setting_menu" background-color="#4c4c4c" text-color="#fff" active-text-color="#ffd04b">
            <el-menu-item index="/setting/system" @click="saveNavState('/setting/system')">
              <i class="el-icon-setting"></i>
              <span slot="title">系统信息</span>
            </el-menu-item>
            <el-menu-item index="/setting/camera" @click="saveNavState('/setting/camera')">
              <i class="el-icon-camera"></i>
              <span slot="title">相机设置</span>
            </el-menu-item>
            <el-menu-item index="/setting/ai" @click="saveNavState('/setting/ai')">
              <i class="el-icon-s-help"></i>
              <span slot="title">智能设置</span>
            </el-menu-item>
            <!-- <el-menu-item index="/setting/video" @click="saveNavState('/setting/video')">
              <i class="el-icon-caret-right"></i>
              <span slot="title">视频设置</span>
            </el-menu-item>
            <el-menu-item index="/setting/overlay" @click="saveNavState('/setting/overlay')">
              <i class="el-icon-menu"></i>
              <span slot="title">图层叠加</span>
            </el-menu-item> -->
            <el-menu-item index="/setting/storage" @click="saveNavState('/setting/storage')">
              <i class="el-icon-film"></i>
              <span slot="title">存储设置</span>
            </el-menu-item>
            <el-menu-item index="/setting/playback" @click="saveNavState('/setting/playback')">
              <i class="el-icon-video-play"></i>
              <span slot="title">录像回放</span>
            </el-menu-item>
          </el-menu>
        </div>
      </el-aside>
      <el-main>
        <router-view></router-view>
      </el-main>
    </el-container>
  </div>
</template>

<script>
export default {
  data () {
    return {
      activePath: '/setting/system'
    }
  },
  created () {
    var path = window.sessionStorage.getItem('settingActivePath')
    if (path) {
      this.activePath = path
    }
    if (this.$router.path !== this.activePath) {
      this.$router.push(this.activePath)
    }
  },
  methods: {
    saveNavState (activePath) {
      window.sessionStorage.setItem('settingActivePath', activePath)
    }
  }
}
</script>

<style lang="less" scoped>
.setting {
  height: 100%!important;
}
.seting-container {
  height: 100%!important;
}

.el-aside {
  background-color: #4c4c4c!important;

  .left-container {
    // height: 800px;
    background-color: #4c4c4c!important;
    .el-menu {
      height: 100%!important;
      border: none;
    }
  }
}
.el-main {
  height: 100%;
  background-color: #eaedf1;
}
</style>
