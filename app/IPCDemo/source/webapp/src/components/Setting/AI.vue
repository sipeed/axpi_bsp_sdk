<template>
  <div>
    <div style="margin-bottom: 20px;">
      <el-button type="primary" @click="onSubmit">修改</el-button>
    </div>
    <el-tabs v-model="activeName" @tab-click="handleClick">
      <el-tab-pane label="智能选项" name="options">
        <el-form ref="formAiRef" :rules="formAiRules" :model="formAi" label-width="100px" label-position="left" size="medium">
          <el-form-item label="智能开启:">
            <el-switch v-model="formAi.ai_attr.ai_enable"></el-switch>
          </el-form-item>
          <el-form-item label="智能模式:">
            <el-select v-model="formAi.ai_attr.detect_model" :disabled="true">
              <el-option v-for="item in detect_modes" :key="item.label" :label="formAi.ai_attr.detect_only ? item.label + '（无推图）' : item.label":value="item.value"> </el-option>
            </el-select>
          </el-form-item>
          <el-form-item label="检测帧率:">
            <el-select v-model="formAi.ai_attr.detect_fps">
              <el-option v-for="item in Array.from({length:15},(item, index)=> index+1)" :key="item" :label="''+item" :value="item"> </el-option>
            </el-select>
          </el-form-item>
          <el-form-item label="推图策略:">
            <el-select v-model="formAi.ai_attr.push_strategy.push_mode">
              <el-option v-for="item in push_strategy_options" :key="item.label" :label="item.label" :value="item.value"> </el-option>
            </el-select>
          </el-form-item>
          <el-form-item label="" v-show="formAi.ai_attr.push_strategy.push_mode == 'FAST' || formAi.ai_attr.push_strategy.push_mode == 'INTERVAL'" >
            <el-form-item label="间隔时间(ms)">
              <el-select v-model="formAi.ai_attr.push_strategy.push_interval">
                <el-option v-for="item in push_interval_options" :key="item.label" :label="item.label" :value="item.value"> </el-option>
              </el-select>
            </el-form-item>
          </el-form-item>
          <el-form-item label="" v-show="formAi.ai_attr.push_strategy.push_mode == 'FAST' || formAi.ai_attr.push_strategy.push_mode == 'INTERVAL'" >
            <el-form-item label="推图数量">
              <el-select v-model="formAi.ai_attr.push_strategy.push_count">
                <el-option v-for="item in Array.from({length:5},(item, index)=> index+1)" :key="item" :label="''+item" :value="item"> </el-option>
              </el-select>
            </el-form-item>
          </el-form-item>
          <el-form-item label="同帧推图:">
            <el-switch v-model="formAi.ai_attr.push_strategy.push_same_frame"></el-switch>
          </el-form-item>
        </el-form>
      </el-tab-pane>
      <el-tab-pane label="脸人配置" name="facehuman" :disabled="formAi.ai_attr.detect_model == 'hvcfp'">
        <el-form ref="formAiRef" :rules="formAiRules" :model="formAi" label-width="100px" label-position="left" size="medium">
          <el-row>
            <el-col :span="9">
              <el-form-item label="人脸检测:" v-show="formAi.ai_attr.detect_only == false">
                <el-switch v-model="formAi.ai_attr.facehuman.face_detect.enable"></el-switch>
              </el-form-item>
            </el-col>
            <el-col :span="9">
              <el-form-item label="人脸画框:">
                <el-switch v-model="formAi.ai_attr.facehuman.face_detect.draw_rect"></el-switch>
              </el-form-item>
            </el-col>
          </el-row>
          <el-row>
            <el-col :span="9">
              <el-form-item label="人形检测:" v-show="formAi.ai_attr.detect_only == false">
                <el-switch v-model="formAi.ai_attr.facehuman.body_detect.enable"></el-switch>
              </el-form-item>
            </el-col>
            <el-col :span="9">
              <el-form-item label="人形画框:">
                <el-switch v-model="formAi.ai_attr.facehuman.body_detect.draw_rect"></el-switch>
              </el-form-item>
            </el-col>
          </el-row>
          <el-row>
            <el-col :span="9">
              <el-form-item label="人脸识别:" v-show="formAi.ai_attr.detect_only == false">
               <el-switch v-model="formAi.ai_attr.facehuman.face_identify.enable"></el-switch>
              </el-form-item>
            </el-col>
          </el-row>
        </el-form>
      </el-tab-pane>
      <el-tab-pane label="结构化配置" name="hvcfp" :disabled="formAi.ai_attr.detect_model == 'facehuman'">
        <el-form ref="formAiRef" :rules="formAiRules" :model="formAi" label-width="100px" label-position="left" size="medium">
          <el-row>
            <el-col :span="9">
              <el-form-item label="人脸检测:" v-show="formAi.ai_attr.detect_only == false">
                <el-switch v-model="formAi.ai_attr.hvcfp.face_detect.enable"></el-switch>
              </el-form-item>
            </el-col>
            <el-col :span="9">
              <el-form-item label="人脸画框:">
                <el-switch v-model="formAi.ai_attr.hvcfp.face_detect.draw_rect"></el-switch>
              </el-form-item>
            </el-col>
          </el-row>
          <el-row>
            <el-col :span="9">
              <el-form-item label="人形检测:" v-show="formAi.ai_attr.detect_only == false">
                <el-switch v-model="formAi.ai_attr.hvcfp.body_detect.enable"></el-switch>
              </el-form-item>
            </el-col>
            <el-col :span="9">
              <el-form-item label="人形画框:">
                <el-switch v-model="formAi.ai_attr.hvcfp.body_detect.draw_rect"></el-switch>
              </el-form-item>
            </el-col>
          </el-row>
          <el-row>
            <el-col :span="9">
              <el-form-item label="机动车检测:" v-show="formAi.ai_attr.detect_only == false">
                <el-switch v-model="formAi.ai_attr.hvcfp.vechicle_detect.enable"></el-switch>
              </el-form-item>
            </el-col>
            <el-col :span="9">
              <el-form-item label="机动车画框:">
                <el-switch v-model="formAi.ai_attr.hvcfp.vechicle_detect.draw_rect"></el-switch>
              </el-form-item>
            </el-col>
          </el-row>
          <el-row>
            <el-col :span="9">
              <el-form-item label="非机动车检测:" v-show="formAi.ai_attr.detect_only == false">
                <el-switch v-model="formAi.ai_attr.hvcfp.cycle_detect.enable"></el-switch>
              </el-form-item>
            </el-col>
            <el-col :span="9">
              <el-form-item label="非机动车画框:">
                <el-switch v-model="formAi.ai_attr.hvcfp.cycle_detect.draw_rect"></el-switch>
              </el-form-item>
            </el-col>
          </el-row>
          <el-row>
            <el-col :span="9">
              <el-form-item label="车牌检测:" v-show="formAi.ai_attr.detect_only == false">
                <el-switch v-model="formAi.ai_attr.hvcfp.plate_detect.enable"></el-switch>
              </el-form-item>
            </el-col>
            <el-col :span="9">
              <el-form-item label="车牌画框:">
                <el-switch v-model="formAi.ai_attr.hvcfp.plate_detect.draw_rect"></el-switch>
              </el-form-item>
            </el-col>
          </el-row>
          <el-row>
            <el-col :span="9">
              <el-form-item label="车牌识别:" v-show="formAi.ai_attr.detect_only == false">
               <el-switch v-model="formAi.ai_attr.hvcfp.plate_identify.enable"></el-switch>
              </el-form-item>
            </el-col>
          </el-row>
        </el-form>
      </el-tab-pane>
      <el-tab-pane label="事件上报" name="events">
        <el-form ref="formAiRef" :rules="formAiRules" :model="formAi" label-width="100px" label-position="left" size="medium">
          <el-form-item label="移动侦测:">
            <el-switch v-model="formAi.ai_attr.events.motion_detect.enable"></el-switch>
            <el-form-item label="Y阈值(1-255)" v-show="formAi.ai_attr.events.motion_detect.enable">
              <el-slider v-model="formAi.ai_attr.events.motion_detect.threshold_y" :min="1" :max="255" show-input></el-slider>
            </el-form-item>
            <el-form-item label="置信度(1-100)" v-show="formAi.ai_attr.events.motion_detect.enable">
              <el-slider v-model="formAi.ai_attr.events.motion_detect.confidence" :min="1" :max="100" show-input></el-slider>
            </el-form-item>
          </el-form-item>
          <el-form-item label="遮挡侦测:">
            <el-switch v-model="formAi.ai_attr.events.occlusion_detect.enable"></el-switch>
            <el-form-item label="Y阈值(1-255)" v-show="formAi.ai_attr.events.occlusion_detect.enable">
              <el-slider v-model="formAi.ai_attr.events.occlusion_detect.threshold_y" :min="1" :max="255" show-input></el-slider>
            </el-form-item>
            <el-form-item label="置信度(1-100)" v-show="formAi.ai_attr.events.occlusion_detect.enable">
              <el-slider v-model="formAi.ai_attr.events.occlusion_detect.confidence" :min="1" :max="100" show-input></el-slider>
            </el-form-item>
          </el-form-item>
        </el-form>
      </el-tab-pane>
    </el-tabs>
  </div>
</template>

<script>
export default {
  data () {
    return {
      activeName: 'options',
      detect_modes: [
        {
          label: '脸人模式',
          value: 'facehuman'
        },
        {
          label: '结构化模式',
          value: 'hvcfp'
        }
      ],
      push_strategy_options: [
        {
          label: '最快推图',
          value: 'FAST'
        },
        {
          label: '间隔推图',
          value: 'INTERVAL'
        },
        {
          label: '最优推图',
          value: 'BEST_FRAME'
        }
      ],
      push_interval_options: [
        {
          label: '1000',
          value: 1000
        },
        {
          label: '2000',
          value: 2000
        },
        {
          label: '3000',
          value: 3000
        },
        {
          label: '5000',
          value: 5000
        },
        {
          label: '10000',
          value: 10000
        },
        {
          label: '20000',
          value: 20000
        }
      ],
      formAi: {
        ai_attr: {
          ai_enable: true,
          detect_model: 'hvcfp',
          detect_fps:15,
          push_strategy: {
            push_mode: 'FAST',
            push_interval: 2000,
            push_count: 1,
            push_same_frame: true
          },
          detect_only: false,
          facehuman: {
            face_detect: {
              enable: true,
              draw_rect: true
            },
            body_detect: {
              enable: true,
              draw_rect: true
            },
            face_identify: {
              enable: true
            }
          },
          hvcfp: {
            face_detect: {
              enable: true,
              draw_rect: true
            },
            body_detect: {
              enable: true,
              draw_rect: true
            },
            vechicle_detect: {
              enable: true,
              draw_rect: true
            },
            cycle_detect: {
              enable: true,
              draw_rect: true
            },
            plate_detect: {
              enable: true,
              draw_rect: true
            },
            plate_identify: {
              enable: true
            }
          },
          events: {
            motion_detect: {
              enable: true,
              threshold_y: 50,
              confidence: 80
            },
            occlusion_detect: {
              enable: true,
              threshold_y: 100,
              confidence: 80
            }
          }
        }
      },
      formAiRules: {
        // pending
      }
    }
  },
  created () {
    console.log('ai++')
    this.getInfo()
  },
  methods: {
    handleClick(tab, event) {
      // console.log(tab, event);
    },
    onSubmit () {
      this.$refs.formAiRef.validate(async valid => {
        if (!valid) return false
        try {
          const { data: res } = await this.$http.post('setting/ai', this.formAi)
          console.log('ai post return: ', res)
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
        const { data: res } = await this.$http.get('setting/ai')
        console.log('ai get return: ', res)
        if (res.meta.status === 200) {
          this.formAi.ai_attr.ai_enable = res.data.ai_attr.ai_enable
          this.formAi.ai_attr.detect_model = res.data.ai_attr.detect_model
          this.formAi.ai_attr.detect_fps = res.data.ai_attr.detect_fps
          this.formAi.ai_attr.push_strategy.push_mode = res.data.ai_attr.push_strategy.push_mode
          this.formAi.ai_attr.push_strategy.push_interval = res.data.ai_attr.push_strategy.push_interval
          this.formAi.ai_attr.push_strategy.push_count = res.data.ai_attr.push_strategy.push_count
          this.formAi.ai_attr.push_strategy.push_same_frame = res.data.ai_attr.push_strategy.push_same_frame

          this.formAi.ai_attr.detect_only = res.data.ai_attr.detect_only
          if (this.formAi.ai_attr.detect_model === 'facehuman') {
            this.formAi.ai_attr.facehuman.face_detect.enable = res.data.ai_attr.facehuman.face_detect.enable
            this.formAi.ai_attr.facehuman.face_detect.draw_rect = res.data.ai_attr.facehuman.face_detect.draw_rect
            this.formAi.ai_attr.facehuman.body_detect.enable = res.data.ai_attr.facehuman.body_detect.enable
            this.formAi.ai_attr.facehuman.body_detect.draw_rect = res.data.ai_attr.facehuman.body_detect.draw_rect
            this.formAi.ai_attr.facehuman.face_identify.enable = res.data.ai_attr.facehuman.face_identify.enable
          } else {
            this.formAi.ai_attr.hvcfp.face_detect.enable = res.data.ai_attr.hvcfp.face_detect.enable
            this.formAi.ai_attr.hvcfp.face_detect.draw_rect = res.data.ai_attr.hvcfp.face_detect.draw_rect
            this.formAi.ai_attr.hvcfp.body_detect.enable = res.data.ai_attr.hvcfp.body_detect.enable
            this.formAi.ai_attr.hvcfp.body_detect.draw_rect = res.data.ai_attr.hvcfp.body_detect.draw_rect
            this.formAi.ai_attr.hvcfp.vechicle_detect.enable = res.data.ai_attr.hvcfp.vechicle_detect.enable
            this.formAi.ai_attr.hvcfp.vechicle_detect.draw_rect = res.data.ai_attr.hvcfp.vechicle_detect.draw_rect
            this.formAi.ai_attr.hvcfp.cycle_detect.enable = res.data.ai_attr.hvcfp.cycle_detect.enable
            this.formAi.ai_attr.hvcfp.cycle_detect.draw_rect = res.data.ai_attr.hvcfp.cycle_detect.draw_rect
            this.formAi.ai_attr.hvcfp.plate_detect.enable = res.data.ai_attr.hvcfp.plate_detect.enable
            this.formAi.ai_attr.hvcfp.plate_detect.draw_rect = res.data.ai_attr.hvcfp.plate_detect.draw_rect
            this.formAi.ai_attr.hvcfp.plate_identify.enable = res.data.ai_attr.hvcfp.plate_identify.enable
          }

          this.formAi.ai_attr.events.motion_detect.enable = res.data.ai_attr.events.motion_detect.enable
          this.formAi.ai_attr.events.motion_detect.threshold_y = res.data.ai_attr.events.motion_detect.threshold_y
          this.formAi.ai_attr.events.motion_detect.confidence = res.data.ai_attr.events.motion_detect.confidence
          this.formAi.ai_attr.events.occlusion_detect.enable = res.data.ai_attr.events.occlusion_detect.enable
          this.formAi.ai_attr.events.occlusion_detect.threshold_y = res.data.ai_attr.events.occlusion_detect.threshold_y
          this.formAi.ai_attr.events.occlusion_detect.confidence = res.data.ai_attr.events.occlusion_detect.confidence
        }
      } catch (error) {
        this.$message.error('获取信息失败')
      }
    }
  }
}
</script>

<style lang="less" scoped>
.el-form {
  width: 500px!important;
}
.el-input {
  width: 100%!important;
}
.el-select {
  width: 100%!important;
}
</style>
