<template>
  <div>
    <el-form ref="formCameraRef" :rules="formCameraRules" :model="formCamera" label-width="220px">
      <el-form-item label="工作模式:">
        <el-select v-model="formCamera.camera_attr.sns_work_mode">
          <el-option v-for="item in sns_mode_options" :key="item.label" :label="item.label" :value="item.value"> </el-option>
        </el-select>
      </el-form-item>
      <el-form-item label="旋转:">
        <el-select v-model="formCamera.camera_attr.rotation">
          <el-option v-for="item in rotation_options" :key="item.label" :label="item.label" :value="item.value"> </el-option>
        </el-select>
      </el-form-item>
      <el-form label-width="220px">
        <el-row>
          <el-col :span="3">
            <el-form-item label="镜像:">
              <el-switch v-model="formCamera.camera_attr.mirror"></el-switch>
            </el-form-item>
          </el-col>
          <el-col :span="3">
            <el-form-item label="翻转:">
              <el-switch v-model="formCamera.camera_attr.flip"></el-switch>
            </el-form-item>
          </el-col>
        </el-row>
      </el-form>
      <el-form-item label="日夜模式:">
        <el-select v-model="formCamera.camera_attr.daynight">
          <el-option v-for="item in daynight_options" :key="item.label" :label="item.label" :value="item.value"> </el-option>
        </el-select>
      </el-form-item>
      <el-form-item label="开启NR:">
        <el-switch v-model="formCamera.camera_attr.nr_mode"></el-switch>
      </el-form-item>
      <el-form-item label="开启EIS:">
        <el-switch v-model="formCamera.camera_attr.eis" :disabled="!formCamera.camera_attr.eis_support"></el-switch>
      </el-form-item>
      <el-form-item label="相机帧率:">
         <el-select v-model="formCamera.camera_attr.framerate">
          <el-option v-for="item in framerate_options" :key="item.label" :label="item.label" :value="item.value"> </el-option>
        </el-select>
      </el-form-item>
      <el-form-item>
        <el-button type="primary" @click="onSubmit">修改</el-button>
      </el-form-item>
    </el-form>
  </div>
</template>

<script>
export default {
  data () {
    return {
      sns_mode_options: [
        {
          label: 'SDR',
          value: 0
        },
        {
          label: 'HDR',
          value: 1
        }
      ],
      rotation_options: [
        {
          label: '0°',
          value: 0
        },
        {
          label: '90°',
          value: 1
        },
        {
          label: '180°',
          value: 2
        },
        {
          label: '270°',
          value: 3
        }
      ],
      daynight_options: [
        {
          label: '日间模式',
          value: 0
        },
        {
          label: '夜间模式',
          value: 1
        }
      ],
      framerate_options: [
        {
          label: '25',
          value: 25
        },
        {
          label: '30',
          value: 30
        }
      ],
      formCamera: {
        camera_attr: {
          sns_work_mode: 0,
          rotation: 0,
          mirror: false,
          flip: false,
          framerate:25,
          daynight: 0,
          nr_mode: true,
          eis_support: false,
          eis: false
        },
        framerate_opts: [
          25,
          30
        ]
      },
      formCameraRules: {
        // pending
      }
    }
  },
  created () {
    console.log('camera++')
    this.getInfo()
  },
  methods: {
    onSubmit () {
      this.$refs.formCameraRef.validate(async valid => {
        if (!valid) return false
        try {
          const { data: res } = await this.$http.post('setting/camera', this.formCamera)
          console.log('camera get return: ', res)
          if (res.meta.status === 200) {
            this.$message.success('修改成功')
          } else {
            this.$message.error('修改失败')
          }
        } catch (error) {
          this.$message.error('修改失败')
        }
      })
    },
    async getInfo () {
      try {
        const { data: res } = await this.$http.get('setting/camera')
        console.log('camera get return: ', res)
        if (res.meta.status === 200) {
          this.formCamera.camera_attr.sns_work_mode = res.data.camera_attr.sns_work_mode
          this.formCamera.camera_attr.rotation = res.data.camera_attr.rotation
          this.formCamera.camera_attr.mirror = res.data.camera_attr.mirror
          this.formCamera.camera_attr.flip = res.data.camera_attr.flip
          this.formCamera.camera_attr.daynight = res.data.camera_attr.daynight
          this.formCamera.camera_attr.framerate = res.data.camera_attr.framerate
          this.formCamera.camera_attr.nr_mode = res.data.camera_attr.nr_mode
          this.formCamera.camera_attr.eis_support = res.data.camera_attr.eis_support
          this.formCamera.camera_attr.eis = res.data.camera_attr.eis
          this.formCamera.framerate_opts = res.data.framerate_opts
          var fps = []
          for (let index=0; index < this.formCamera.framerate_opts.length; index++) {
            fps.push({lable: ''+this.formCamera.framerate_opts[index], value: this.formCamera.framerate_opts[index]})
          }
          this.framerate_options = fps
        }
      } catch (error) {
        this.$message.error('获取信息失败')
      }
    }
  }
}
</script>

<style lang="less" scoped>
.el-input {
   width: 200px!important;
}
</style>
