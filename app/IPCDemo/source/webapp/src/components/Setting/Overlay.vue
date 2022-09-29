<template>
  <div>
    <div class="overlay-preview-container"></div>
    <div>
      <el-form ref="formOverlayRef" :rules="formOverlayRules" :model="formOverlay" label-width="100px">
        <el-form-item label="视频源ID">
          <el-select v-model="formOverlay.video_src_info.id" @change="onChangeSrcID">
            <el-option v-for="item in formOverlay.video_src_opts" :key="item.value" :label="item.label" :value="item.value"></el-option>
          </el-select>
        </el-form-item>
        <el-form-item label="画线">
          <el-switch v-model="formOverlay.drawLine.enable"></el-switch>
        </el-form-item>
        <el-form-item label="" label-width="20px">
          <el-form-item label="x1:" class="inline">
            <el-input-number controls-position="right" :min="0" :max="1919" v-model="formOverlay.drawLine.ptBegin[0]" :disabled="!formOverlay.drawLine.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="y1:" class="inline">
            <el-input-number controls-position="right" :min="0" :max="1087" v-model="formOverlay.drawLine.ptBegin[1]" :disabled="!formOverlay.drawLine.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="x2:" class="inline">
            <el-input-number controls-position="right" :min="0" :max="1919" v-model="formOverlay.drawLine.ptEnd[0]" :disabled="!formOverlay.drawLine.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="y2:" class="inline">
            <el-input-number controls-position="right" :min="0" :max="1087" v-model="formOverlay.drawLine.ptEnd[1]" :disabled="!formOverlay.drawLine.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="线宽:" class="inline">
            <el-input-number controls-position="right" :min="2" :max="16" v-model="formOverlay.drawLine.lineWidth" :disabled="!formOverlay.drawLine.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="颜色:" class="inline">
            <el-select v-model="formOverlay.drawLine.color" :disabled="!formOverlay.drawLine.enable">
              <el-option v-for="item in colorOptions" :key="item.label" :label="item.label" :value="item.value"> </el-option>
            </el-select>
          </el-form-item>
        </el-form-item>
        <el-form-item label="画矩形框">
          <el-switch v-model="formOverlay.drawRect.enable"></el-switch>
        </el-form-item>
        <el-form-item label="" label-width="20px">
          <el-form-item label="x:" class="inline">
            <el-input-number controls-position="right" :min="0" :max="1919" v-model="formOverlay.drawRect.x" :disabled="!formOverlay.drawRect.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="y:" class="inline">
            <el-input-number controls-position="right" :min="0" :max="1087" v-model="formOverlay.drawRect.y" :disabled="!formOverlay.drawRect.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="宽:" class="inline">
            <el-input-number controls-position="right" :min="8" :max="256" v-model="formOverlay.drawRect.w" :disabled="!formOverlay.drawRect.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="高:" class="inline">
            <el-input-number controls-position="right" :min="8" :max="256" v-model="formOverlay.drawRect.h" :disabled="!formOverlay.drawRect.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="线宽:" class="inline">
            <el-input-number controls-position="right" :min="2" :max="16" v-model="formOverlay.drawRect.lineWidth" :disabled="!formOverlay.drawRect.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="颜色:" class="inline">
            <el-select v-model="formOverlay.drawRect.color" :disabled="!formOverlay.drawRect.enable">
              <el-option v-for="item in colorOptions" :key="item.label" :label="item.label" :value="item.value"> </el-option>
            </el-select>
          </el-form-item>
          <el-form-item label="填充:" class="inline">
            <el-switch v-model="formOverlay.drawRect.solid" :disabled="!formOverlay.drawRect.enable"></el-switch>
          </el-form-item>
        </el-form-item>
        <el-form-item label="画凸四边形">
          <el-switch v-model="formOverlay.drawPolygon.enable"></el-switch>
        </el-form-item>
        <el-form-item label="" label-width="20px">
          <el-form-item label="x1:" class="inline">
            <el-input-number controls-position="right" :min="0" :max="1919" v-model="formOverlay.drawPolygon.pt0[0]" :disabled="!formOverlay.drawPolygon.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="y1:" class="inline">
            <el-input-number controls-position="right" :min="0" :max="1087" v-model="formOverlay.drawPolygon.pt0[1]" :disabled="!formOverlay.drawPolygon.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="x2:" class="inline">
            <el-input-number controls-position="right" :min="0" :max="1919" v-model="formOverlay.drawPolygon.pt1[0]" :disabled="!formOverlay.drawPolygon.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="y2:" class="inline">
            <el-input-number controls-position="right" :min="0" :max="1087" v-model="formOverlay.drawPolygon.pt1[1]" :disabled="!formOverlay.drawPolygon.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="x3:" class="inline">
            <el-input-number controls-position="right" :min="0" :max="1919" v-model="formOverlay.drawPolygon.pt2[0]" :disabled="!formOverlay.drawPolygon.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="y3:" class="inline">
            <el-input-number controls-position="right" :min="0" :max="1087" v-model="formOverlay.drawPolygon.pt2[1]" :disabled="!formOverlay.drawPolygon.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="x4:" class="inline">
            <el-input-number controls-position="right" :min="0" :max="1919" v-model="formOverlay.drawPolygon.pt3[0]" :disabled="!formOverlay.drawPolygon.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="y4:" class="inline">
            <el-input-number controls-position="right" :min="0" :max="1087" v-model="formOverlay.drawPolygon.pt3[1]" :disabled="!formOverlay.drawPolygon.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="线宽:" class="inline">
            <el-input-number controls-position="right" :min="2" :max="16" v-model="formOverlay.drawPolygon.lineWidth" :disabled="!formOverlay.drawPolygon.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="颜色:" class="inline">
            <el-select v-model="formOverlay.drawPolygon.color" :disabled="!formOverlay.drawPolygon.enable">
              <el-option v-for="item in colorOptions" :key="item.label" :label="item.label" :value="item.value"> </el-option>
            </el-select>
          </el-form-item>
          <el-form-item label="填充:" class="inline">
            <el-switch v-model="formOverlay.drawPolygon.solid" :disabled="!formOverlay.drawPolygon.enable"></el-switch>
          </el-form-item>
        </el-form-item>
        <el-form-item label="画马赛克">
          <el-switch v-model="formOverlay.drawMosaic.enable"></el-switch>
        </el-form-item>
        <el-form-item label="" label-width="20px">
          <el-form-item label="x:" class="inline">
            <el-input-number controls-position="right" :min="0" :max="1919" v-model="formOverlay.drawMosaic.x" :disabled="!formOverlay.drawMosaic.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="y:" class="inline">
            <el-input-number controls-position="right" :min="0" :max="1087" v-model="formOverlay.drawMosaic.y" :disabled="!formOverlay.drawMosaic.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="宽:" class="inline">
            <el-input-number controls-position="right" :min="8" :max="256" v-model="formOverlay.drawMosaic.w" :disabled="!formOverlay.drawMosaic.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="高:" class="inline">
            <el-input-number controls-position="right" :min="8" :max="256" v-model="formOverlay.drawMosaic.h" :disabled="!formOverlay.drawMosaic.enable"></el-input-number>
          </el-form-item>
          <el-form-item label="块:" class="inline">
            <el-select v-model="formOverlay.drawMosaic.blockSize" :disabled="!formOverlay.drawMosaic.enable">
            <el-option v-for="item in blockOptions" :key="item" :label="item" :value="item"> </el-option>
          </el-select>
          </el-form-item>
        </el-form-item>
        <el-form-item label="旋转">
          <el-switch v-model="formOverlay.rotation.enable"></el-switch>
        </el-form-item>
        <el-form-item label="">
          <el-select v-model="formOverlay.rotation.rotation" :disabled="!formOverlay.rotation.enable">
            <el-option v-for="item in rotationOptions" :key="item" :label="item" :value="item"> </el-option>
          </el-select>
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
      blockOptions: [8, 16, 64],
      rotationOptions: [0, 90, 180, 270],
      colorOptions: [
        {
          label: 'Green',
          value: '0x000000'
        },
        {
          label: 'Red',
          value: '0x0000FF'
        },
        {
          label: 'Blue',
          value: '0x00FF00'
        },
        {
          label: 'Purple',
          value: '0x00FFFF'
        },
        {
          label: 'Yellow',
          value: '0xFF00FF'
        },
        {
          label: 'Black',
          value: '0x008080'
        },
        {
          label: 'White',
          value: '0xFF8080'
        },
        {
          label: 'Gray',
          value: '0x808080'
        }
      ],
      formOverlay: {
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
        video_src_info: {
          id: 0
        },
        drawLine: {
          enable: false,
          ptBegin: [0, 0],
          ptEnd: [0, 0],
          color: '0xFF8080',
          lineWidth: 2
        },
        drawRect: {
          enable: false,
          x: 0,
          y: 0,
          w: 0,
          h: 0,
          solid: true,
          color: '0xFF8080',
          lineWidth: 2
        },
        drawPolygon: {
          enable: false,
          pt0: [0, 0],
          pt1: [0, 0],
          pt2: [0, 0],
          pt3: [0, 0],
          solid: true,
          color: '0xFF8080',
          lineWidth: 2
        },
        drawMosaic: {
          enable: false,
          x: 0,
          y: 0,
          w: 0,
          h: 0,
          blockSize: 8
        },
        rotation: {
          enable: false,
          rotation: 90
        }
      },
      formOverlayRules: {
      }
    }
  },
  created () {
    console.log('overlay++')
    this.getInfo()
  },
  methods: {
    async onSubmit () {
      try {
        var _uri = 'setting/overlay_' + this.formOverlay.video_src_info.id
        const { data: res } = await this.$http.post(_uri, this.formOverlay)
        console.log('overlay get return: ', res)
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
        var _uri = 'setting/overlay_' + this.formOverlay.video_src_info.id
        const { data: res } = await this.$http.get(_uri)
        console.log('overlay get return: ', res)
        if (res.meta.status === 200) {
          this.formOverlay.drawLine = res.data.drawLine
          this.formOverlay.drawRect = res.data.drawRect
          this.formOverlay.drawPolygon = res.data.drawPolygon
          this.formOverlay.drawMosaic = res.data.drawMosaic
          this.formOverlay.rotation = res.data.rotation
        }
      } catch (error) {
        this.$message.error('获取信息失败')
      }
    },
    onChangeSrcID(data) {
      this.getInfo()
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
