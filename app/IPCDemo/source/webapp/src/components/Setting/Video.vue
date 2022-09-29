<template>
  <div>
    <div class="video-preview-container"></div>
    <div>
      <el-form ref="formVideoRef" :rules="formVideoRules" :model="formVideo" label-width="100px">
        <el-form-item label="视频源ID">
          <el-select v-model="formVideo.video_src_info.id" @change="onChangeSrcID">
            <el-option v-for="item in formVideo.video_src_opts" :key="item.value" :label="item.label" :value="item.value"></el-option>
          </el-select>
        </el-form-item>
        <el-form-item label="主码流0">
        </el-form-item>
        <el-form-item label="" label-width="20px">
          <el-form-item label="编码类型:" class="inline">
            <el-select v-model="formVideo.video0.encode_type" style="width:100%" disabled="true">
              <el-option v-for="item in formVideo.encode_opts" :key="item.label" :label="item.label" :value="item.value"> </el-option>
            </el-select>
          </el-form-item>
          <el-form-item label="帧率:" class="inline" v-show="false">
            <el-input-number controls-position="right" :min="10" :max="30" v-model="formVideo.video0.frame_rate"></el-input-number>
          </el-form-item>
          <el-form-item label="码率(kbps):" class="inline">
            <el-input-number controls-position="right" :min="500" :max="5000" :step="500" v-model="formVideo.video0.bit_rate" style="width:100%"></el-input-number>
          </el-form-item>
          <el-form-item label="分辨率:" class="inline">
            <el-select v-model="formVideo.video0.resolution_index" style="width:100%">
              <el-option v-for="item in formVideo.res_options_0" :key="item.label" :label="item.label" :value="item.value"> </el-option>
            </el-select>
          </el-form-item>
        </el-form-item>
        <el-form-item label="子码流1">
        </el-form-item>
        <el-form-item label="" label-width="20px">
          <el-form-item label="编码类型:" class="inline">
            <el-select v-model="formVideo.video1.encode_type" style="width:100%" disabled="true">
              <el-option v-for="item in formVideo.encode_opts" :key="item.label" :label="item.label" :value="item.value"> </el-option>
            </el-select>
          </el-form-item>
          <el-form-item label="帧率:" class="inline" v-show="false">
            <el-input-number controls-position="right" :min="10" :max="30" v-model="formVideo.video1.frame_rate"></el-input-number>
          </el-form-item>
          <el-form-item label="码率(kbps):" class="inline">
            <el-input-number controls-position="right" :min="500" :max="100000" :step="500" v-model="formVideo.video1.bit_rate" style="width:100%"></el-input-number>
          </el-form-item>
          <el-form-item label="分辨率:" class="inline">
            <el-select v-model="formVideo.video1.resolution_index" style="width:100%">
              <el-option v-for="item in formVideo.res_options_1" :key="item.label" :label="item.label" :value="item.value"> </el-option>
            </el-select>
          </el-form-item>
        </el-form-item>
        <el-form-item label="子码流2">
        </el-form-item>
        <el-form-item label="" label-width="20px">
          <el-form-item label="编码类型:" class="inline">
            <el-select v-model="formVideo.video2.encode_type" style="width:100%" disabled="true">
              <el-option v-for="item in formVideo.encode_opts" :key="item.label" :label="item.label" :value="item.value"> </el-option>
            </el-select>
          </el-form-item>
          <el-form-item label="帧率:" class="inline" v-show="false">
            <el-input-number controls-position="right" :min="10" :max="30" v-model="formVideo.video2.frame_rate"></el-input-number>
          </el-form-item>
          <el-form-item label="码率(kbps):" class="inline">
            <el-input-number controls-position="right" :min="500" :max="5000" :step="500" v-model="formVideo.video2.bit_rate" style="width:100%"></el-input-number>
          </el-form-item>
          <el-form-item label="分辨率:" class="inline">
            <el-select v-model="formVideo.video2.resolution_index" style="width:100%">
              <el-option v-for="item in formVideo.res_options_2" :key="item.label" :label="item.label" :value="item.value"> </el-option>
            </el-select>
          </el-form-item>
        </el-form-item>
        <el-form-item>
          <el-button type="primary" @click="onSubmit">修改</el-button>
        </el-form-item>
      </el-form>
    </div>
  </div>
</template>

<script>
export default {
  data () {
    return {
      formVideo: {
        video_src_opts:[
          {
            label: '视频源0',
            value: 0
          },
          {
            label: '视频源1',
            value: 1
          }
        ],
        encode_opts:[
          {
            label: 'H264',
            value: 0
          },
          {
            label: 'MJPEG',
            value: 1
          }
        ],
        res_opts: [],
        res_options_0: [],
        res_options_1: [],
        res_options_2: [],
        video_src_info: {
          id: 0
        },
        video0: {
          encode_type: 0,
          frame_rate: 0,
          bit_rate: 0,
          resolution_index: 0
        },
        video1: {
          encode_type: 1,
          frame_rate: 0,
          bit_rate: 0,
          resolution_index: 0
        },
        video2: {
          encode_type: 0,
          frame_rate: 0,
          bit_rate: 0,
          resolution_index: 0
        }
      },
      formVideoRules: {
      }
    }
  },
  created () {
    console.log('video++')
    this.getInfo()
  },
  methods: {
    async onSubmit () {
      try {
        var objData = {}
        objData = JSON.parse(JSON.stringify(this.formVideo))

        objData.video0.bit_rate = this.formVideo.video0.bit_rate * 1000
        objData.video1.bit_rate = this.formVideo.video1.bit_rate * 1000
        objData.video2.bit_rate = this.formVideo.video2.bit_rate * 1000

        var _uri = 'setting/video_' + this.formVideo.video_src_info.id
        const { data: res } = await this.$http.post(_uri, objData)
        console.log('video get return: ', res)
        if (res.meta.status === 200) {
          this.$message.success('修改成功')
        } else {
          this.$message.success('修改失败')
        }
      } catch (error) {
        this.$message.error('修改失败')
      }
    },
    async getInfo () {
      try {
        var _uri = 'setting/video_' + this.formVideo.video_src_info.id
        const { data: res } = await this.$http.get(_uri)
        console.log('video get return: ', res)
        if (res.meta.status === 200) {
          this.formVideo.res_opts = res.data.res_opts
          this.formVideo.video0 = res.data.video0
          this.formVideo.video1 = res.data.video1
          this.formVideo.video2 = res.data.video2

          this.formVideo.video0.bit_rate = this.formVideo.video0.bit_rate / 1000
          this.formVideo.video1.bit_rate = this.formVideo.video1.bit_rate / 1000
          this.formVideo.video2.bit_rate = this.formVideo.video2.bit_rate / 1000

          this.init_resolution_options()
        }
      } catch (error) {
        this.$message.error('获取信息失败')
      }
    },
    onChangeSrcID(data) {
      this.getInfo()
    },
    init_resolution_options()
    {
      this.formVideo.res_options_0 = []
      this.formVideo.res_options_1 = []
      this.formVideo.res_options_2 = []

      for (var i = 0;i < this.formVideo.res_opts.length;i++)
      {
        var _item = this.get_resolution_item(this.formVideo.res_opts[i] + '')
        if (i === 0) {
          this.formVideo.res_options_0.push(_item)
        }
        else {
          this.formVideo.res_options_0.push(_item)
          this.formVideo.res_options_1.push(_item)
          if (i != 1) {
            this.formVideo.res_options_2.push(_item)
          }
        }
      }
    },
    get_resolution_item(index)
    {
      var _item = {}
      switch(index + '') {
        case '0':
          _item = {
            label: '3840 x 2160',
            value: 0
          }
          break
        case '1':
          _item = {
            label: '2688 x 1520',
            value: 1
          }
          break
        case '2':
          _item = {
            label: '1920 x 1080',
            value: 2
          }
          break
        case '3':
          _item = {
            label: '1280 x 720',
            value: 3
          }
          break
        case '4':
          _item = {
            label: '640 x 480',
            value: 4
          }
          break
        default:
          break
      }
      return _item
    }
  }
}
</script>

<style lang="less" scoped>
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
