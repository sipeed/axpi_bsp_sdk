<template>
  <div class="total_container">
    <el-container :class="mode_option == 'ai' ? 'preview_outside_container' : 'preview_outside_container_normal'">
      <el-container class="preview_main_container">
        <el-main class="preview_capture_left_container">
          <el-container class="preview_container">
            <el-main ref="axMain" class="video_container">
              <video id="myVideo" ref="axVideoRef" class="axVideo" autoplay muted playsinline oncontextmenu="return false;">
              </video>
            </el-main>
            <el-footer class="video_controls_container" height="30px">
              <div class="left_controls_container">
                <div class="size_option">
                  <el-dropdown @command="changeSizeOption" size="mini" placement="top" trigger="click">
                    <el-button size="mini" class="el-dropdown-link" type="text">{{sizeOptions[sizeOptionInd].label}}</el-button>
                    <el-dropdown-menu slot="dropdown" :append-to-body="false">
                      <el-dropdown-item :command="item.id + ''" :icon="item.icon" v-for="item in sizeOptions" :key="item.id">{{item.label}}</el-dropdown-item>
                    </el-dropdown-menu>
                  </el-dropdown>
                </div>
                <div>
                  <el-dropdown @command="changeStream" size="mini" placement="top" trigger="click">
                    <el-button size="mini" class="el-dropdown-link" type="text">{{streamOptions[streamOptionInd].label}}</el-button>
                    <el-dropdown-menu slot="dropdown" :append-to-body="false">
                      <el-dropdown-item :command="item.id + ''" :icon="item.icon" v-for="item in streamOptions" :key="item.id">{{item.label}}</el-dropdown-item>
                    </el-dropdown-menu>
                  </el-dropdown>
                </div>
                <div class="size_info">
                  <span>{{this.enc_info}}</span>
                </div>
              </div>
              <div class="right_controls_container">
                <div class="draw_option">
                  <el-tooltip effect="dark" :content="'抓拍'" placement="top-end" :enterable="false">
                    <el-button size="mini" :class="{'draw_button_enable':true, 'draw_button_disable':false}" icon="el-icon-camera-solid" @click="onSnapshot"></el-button>
                  </el-tooltip>
                </div>
                <div class="draw_option">
                  <el-tooltip effect="dark" :content="'录入人像信息'" placement="top-end" :enterable="false">
                    <el-button size="mini" :class="{'draw_button_enable':true, 'draw_button_disable':false}" icon="el-icon-user-solid" @click="recordFaceinfo" v-show="isSearchImg && detect_mode == 'facehuman'"></el-button>
                  </el-tooltip>
                </div>
                <div class="draw_option">
                  <el-tooltip effect="dark" :content="!isRecStart ? '开启录像' : '停止录像'" placement="top-end" :enterable="false">
                    <el-button size="mini" :disabled="rec_enable === false" :class="{'draw_button_enable':!isRecStart, 'draw_button_disable': isRecStart}" icon="el-icon-video-camera-solid" @click="onRecording"></el-button>
                  </el-tooltip>
                </div>
                <div class="full_option">
                  <el-tooltip effect="dark" :content="isFullScreen ? '退出全屏' : '进入全屏'" placement="top-end" :enterable="false">
                    <el-button size="mini" class="full_button" :icon="screenIcon" @click="doScreen"></el-button>
                  </el-tooltip>
                </div>
              </div>
            </el-footer >
          </el-container>
        </el-main>
        <el-aside class="side_identify_events_container" width="320px" v-show="mode_option == 'ai'" >
          <el-container class="identify_events_container">
            <el-header class="identify_container">
              <el-row class="image_info_container">
                <el-col :span="10" class="image_col">
                  <!--
                  <el-image class="image-large" :src="currentImg.src" :preview-src-list="[currentImg.src]" fit="fill">
                    <div slot="error" class="image-slot-large">
                      <i class="el-icon-picture-outline"></i>
                    </div>
                  </el-image>
                  <el-image-viewer v-if="showViewer && currentImg.src != ''" :on-close="closeViewer" :url-list="[currentImg.src]"/>
                  -->
                  <div class="image-large">
                    <i class="el-icon-picture-outline" v-if="currentImg.src == ''"></i>
                    <img class="image-large" v-if="currentImg.src != ''" :src="currentImg.src" crossOrigin="anonymous" @click="look(currentImg)"></img>
                  </div>
                </el-col>
                <el-col :span="14" class="info_col">
                  <div class="info_container">
                    <div class="info_text" v-show="detect_mode == 'facehuman'">
                      性别：    {{currentImg.info.gender}}
                    </div>
                    <div class="info_text"  v-show="detect_mode == 'facehuman'">
                      年龄：    {{currentImg.info.age}}
                    </div>
                    <div class="info_text"  v-show="detect_mode == 'facehuman'">
                      口罩：    {{currentImg.info.mask}}
                    </div>
                    <div class="info_text"  v-show="detect_mode == 'facehuman'">
                      识别：    {{currentImg.info.info}}
                    </div>
                    <div class="info_text"  v-show="detect_mode == 'hvcfp'">
                      车牌号码：{{currentImg.info.number}}
                    </div>
                    <div class="info_text"  v-show="detect_mode == 'hvcfp'">
                      车牌颜色：{{currentImg.info.color}}
                    </div>
                  </div>
                </el-col>
              </el-row>
            </el-header>
            <el-main class="events_container">
              <el-table :header-cell-style="{background:'#2C2C2C', color:'#B0B0B0',border:'none',fontSize:'10px', padding:'4px'}" :cell-style="{padding:'4px'}" :row-style="{height:'16px'}" style="font-size: 10px" height="150" class="events_table" :data="eventList" highlight-current-row>
                <el-table-column type="index" label="#" width="60"></el-table-column>
                <el-table-column property="date"  label="发生时间" width="125"> </el-table-column>
                <el-table-column property="type"  label="事件类型" width="125"> </el-table-column>
              </el-table>
            </el-main>
          </el-container>
        </el-aside>
      </el-container>
      <el-footer class="hvcfp_img_container" v-show="detect_mode == 'hvcfp' && mode_option == 'ai'">
        <el-row class="img_list_top">
          <el-col :span="2">
            <div class="type_text">人脸</div>
          </el-col>
          <el-col :span="11" class="hvcfp_img_col">
            <el-col class="hvcfp_img_col" :span="3" v-for="face_ind in image_count/2" :key="face_ind">
               <div class="image-small">
                <i class="el-icon-picture-outline" v-if="pushHCVFPImages.face[face_ind-1].src == ''"></i>
                <img class="image-small" v-if="pushHCVFPImages.face[face_ind-1].src != ''" :src="pushHCVFPImages.face[face_ind-1].src" @click="look(pushHCVFPImages.face[face_ind-1])"></img>
              </div>
            </el-col>
          </el-col>
          <el-col :span="11" class="hvcfp_img_col">
            <el-col class="hvcfp_img_col" :span="3" v-for="face_ind in image_count/2" :key="face_ind">
              <div class="image-small">
                <i class="el-icon-picture-outline" v-if="pushHCVFPImages.face[image_count/2 + face_ind-1].src == ''"></i>
                <img class="image-small" v-if="pushHCVFPImages.face[image_count/2 + face_ind-1].src != ''" :src="pushHCVFPImages.face[image_count/2 + face_ind-1].src" @click="look(pushHCVFPImages.face[image_count/2 + face_ind-1])"></img>
              </div>
            </el-col>
          </el-col>
        </el-row>
        <el-row class="img_list">
          <el-col class="hvcfp_img_col" :span="2">
            <div class="type_text">人形</div>
          </el-col>
          <el-col class="hvcfp_img_col" :span="11">
            <el-col class="hvcfp_img_col" :span="3" v-for="body_ind in image_count/2" :key="body_ind">
              <div class="image-small">
                <i class="el-icon-picture-outline" v-if="pushHCVFPImages.body[body_ind-1].src == ''"></i>
                <img class="image-small" v-if="pushHCVFPImages.body[body_ind-1].src != ''" :src="pushHCVFPImages.body[body_ind-1].src" @click="look(pushHCVFPImages.body[body_ind-1])"></img>
              </div>
            </el-col>
          </el-col>
          <el-col class="hvcfp_img_col" :span="11">
            <el-col class="hvcfp_img_col" :span="3" v-for="body_ind in image_count/2" :key="body_ind">
              <div class="image-small">
                <i class="el-icon-picture-outline" v-if="pushHCVFPImages.body[image_count/2 + body_ind-1].src == ''"></i>
                <img class="image-small" v-if="pushHCVFPImages.body[image_count/2 + body_ind-1].src != ''" :src="pushHCVFPImages.body[image_count/2 + body_ind-1].src" @click="look(pushHCVFPImages.body[image_count/2 + body_ind-1])"></img>
              </div>
            </el-col>
          </el-col>
        </el-row>
        <el-row class="img_list">
          <el-col class="hvcfp_img_col" :span="2">
            <div class="type_text">非机动车</div>
          </el-col>
          <el-col class="hvcfp_img_col" :span="11">
            <el-col class="hvcfp_img_col" :span="3" v-for="cycle_ind in image_count/2" :key="cycle_ind" >
              <div class="image-small">
                <i class="el-icon-picture-outline" v-if="pushHCVFPImages.cycle[cycle_ind-1].src == ''"></i>
                <img class="image-small" v-if="pushHCVFPImages.cycle[cycle_ind-1].src != ''" :src="pushHCVFPImages.cycle[cycle_ind-1].src" @click="look(pushHCVFPImages.cycle[cycle_ind-1])"></img>
              </div>
            </el-col>
          </el-col>
          <el-col class="hvcfp_img_col" :span="11">
            <el-col class="hvcfp_img_col" :span="3" v-for="cycle_ind in image_count/2" :key="cycle_ind" >
              <div class="image-small">
                <i class="el-icon-picture-outline" v-if="pushHCVFPImages.cycle[image_count/2 + cycle_ind-1].src == ''"></i>
                <img class="image-small" v-if="pushHCVFPImages.cycle[image_count/2 + cycle_ind-1].src != ''" :src="pushHCVFPImages.cycle[image_count/2 + cycle_ind-1].src" @click="look(pushHCVFPImages.cycle[image_count/2 + cycle_ind-1])"></img>
              </div>
            </el-col>
          </el-col>
        </el-row>
        <el-row class="img_list">
          <el-col class="hvcfp_img_col" :span="2">
            <div class="type_text">机动车</div>
          </el-col>
          <el-col class="hvcfp_img_col" :span="11">
            <el-col class="hvcfp_img_col" :span="3" v-for="vechicle_ind in image_count/2" :key="vechicle_ind">
              <div class="image-small">
                <i class="el-icon-picture-outline" v-if="pushHCVFPImages.vechicle[vechicle_ind-1].src == ''"></i>
                <img class="image-small" v-if="pushHCVFPImages.vechicle[vechicle_ind-1].src != ''" :src="pushHCVFPImages.vechicle[vechicle_ind-1].src" @click="look(pushHCVFPImages.vechicle[vechicle_ind-1])"></img>
              </div>
            </el-col>
          </el-col>
          <el-col class="hvcfp_img_col" :span="11">
            <el-col class="hvcfp_img_col" :span="3" v-for="vechicle_ind in image_count/2" :key="vechicle_ind">
              <div class="image-small">
                <i class="el-icon-picture-outline" v-if="pushHCVFPImages.vechicle[image_count/2 + vechicle_ind-1].src == ''"></i>
                <img class="image-small" v-if="pushHCVFPImages.vechicle[image_count/2 + vechicle_ind-1].src != ''" :src="pushHCVFPImages.vechicle[image_count/2 + vechicle_ind-1].src" @click="look(pushHCVFPImages.vechicle[image_count/2 + vechicle_ind-1])"></img>
              </div>
            </el-col>
          </el-col>
        </el-row>
        <el-row class="img_list_bottom">
          <el-col class="hvcfp_img_col" :span="2">
            <div class="type_text">车牌</div>
          </el-col>
          <el-col class="hvcfp_img_col" :span="11">
            <el-col class="hvcfp_img_col" :span="3" v-for="plate_ind in image_count/2" :key="plate_ind">
              <div class="image-small">
                <i class="el-icon-picture-outline" v-if="pushHCVFPImages.plate[plate_ind-1].src == ''"></i>
                <img class="image-small" v-if="pushHCVFPImages.plate[plate_ind-1].src != ''" :src="pushHCVFPImages.plate[plate_ind-1].src" @click="look(pushHCVFPImages.plate[plate_ind-1])"></img>
              </div>
            </el-col>
          </el-col>
          <el-col class="hvcfp_img_col" :span="11">
            <el-col class="hvcfp_img_col" :span="3" v-for="plate_ind in image_count/2" :key="plate_ind">
              <div class="image-small">
                <i class="el-icon-picture-outline" v-if="pushHCVFPImages.plate[image_count/2 + plate_ind-1].src == ''"></i>
                <img class="image-small" v-if="pushHCVFPImages.plate[image_count/2 + plate_ind-1].src != ''" :src="pushHCVFPImages.plate[image_count/2 + plate_ind-1].src" @click="look(pushHCVFPImages.plate[image_count/2 + plate_ind-1])"></img>
              </div>
            </el-col>
          </el-col>
        </el-row>
      </el-footer>
      <el-footer class="fh_img_container" v-show="detect_mode == 'facehuman'  && mode_option == 'ai'">
        <el-row class="img_list_top">
          <el-col class="fp_img_col" :span="2">
            <div class="type_text_fh">人脸</div>
          </el-col>
          <el-col class="fp_img_col" :span="11">
            <el-col class="fp_img_col" :span="3"  v-for="face_ind in image_count/2" :key="face_ind">
              <div class="image-medium">
                <i class="el-icon-picture-outline" v-if="pushFHImages.face[face_ind-1].src == ''"></i>
                <img class="image-medium" v-if="pushFHImages.face[face_ind-1].src != ''" :src="pushFHImages.face[face_ind-1].src" @click="look(pushFHImages.face[face_ind-1])"></img>
              </div>
            </el-col>
          </el-col>
          <el-col class="fp_img_col" :span="11">
            <el-col class="fp_img_col" :span="3"  v-for="face_ind in image_count/2" :key="face_ind">
              <div class="image-medium">
                <i class="el-icon-picture-outline" v-if="pushFHImages.face[image_count/2 + face_ind-1].src == ''"></i>
                <img class="image-medium" v-if="pushFHImages.face[image_count/2 + face_ind-1].src != ''" :src="pushFHImages.face[image_count/2 + face_ind-1].src" @click="look(pushFHImages.face[image_count/2 + face_ind-1])"></img>
              </div>
            </el-col>
          </el-col>
        </el-row>
        <el-row class="img_list">
          <el-col class="fp_img_col" :span="2">
            <div class="type_text_fh">人脸</div>
          </el-col>
          <el-col class="fp_img_col" :span="11">
            <el-col class="fp_img_col" :span="3"  v-for="face_ind in image_count/2" :key="face_ind">
              <div class="image-medium">
                <i class="el-icon-picture-outline" v-if="pushFHImages.face[image_count + face_ind - 1].src == ''"></i>
                <img class="image-medium" v-if="pushFHImages.face[image_count + face_ind - 1].src != ''" :src="pushFHImages.face[image_count + face_ind - 1].src" @click="look(pushFHImages.face[image_count + face_ind - 1])"></img>
              </div>
            </el-col>
          </el-col>
          <el-col class="fp_img_col" :span="11">
            <el-col class="fp_img_col" :span="3"  v-for="face_ind in image_count/2" :key="face_ind">
              <div class="image-medium">
                <i class="el-icon-picture-outline" v-if="pushFHImages.face[image_count/2 + image_count + face_ind - 1].src == ''"></i>
                <img class="image-medium" v-if="pushFHImages.face[image_count/2 + image_count + face_ind - 1].src != ''" :src="pushFHImages.face[image_count/2 + image_count + face_ind - 1].src" @click="look(pushFHImages.face[image_count/2 + image_count + face_ind - 1])"></img>
              </div>
            </el-col>
          </el-col>
        </el-row>
        <el-row class="img_list">
          <el-col class="fp_img_col" :span="2">
            <div class="type_text_fh">人形</div>
          </el-col>
          <el-col class="fp_img_col" :span="11">
            <el-col class="fp_img_col" :span="3"  v-for="body_ind in image_count/2" :key="body_ind">
              <div class="image-medium">
                <i class="el-icon-picture-outline" v-if="pushFHImages.body[body_ind-1].src == ''"></i>
                <img class="image-medium" v-if="pushFHImages.body[body_ind-1].src != ''" :src="pushFHImages.body[body_ind-1].src" @click="look(pushFHImages.body[body_ind-1])"></img>
              </div>
            </el-col>
          </el-col>
          <el-col class="fp_img_col" :span="11">
            <el-col class="fp_img_col" :span="3"  v-for="body_ind in image_count/2" :key="body_ind">
              <div class="image-medium">
                <i class="el-icon-picture-outline" v-if="pushFHImages.body[image_count/2 + body_ind-1].src == ''"></i>
                <img class="image-medium" v-if="pushFHImages.body[image_count/2 + body_ind-1].src != ''" :src="pushFHImages.body[image_count/2 + body_ind-1].src" @click="look(pushFHImages.body[image_count/2 + body_ind-1])"></img>
              </div>
            </el-col>
          </el-col>
        </el-row>
        <el-row class="img_list_bottom">
          <el-col class="fp_img_col" :span="2">
            <div class="type_text_fh">人形</div>
          </el-col>
          <el-col class="fp_img_col" :span="11">
            <el-col class="fp_img_col" :span="3"  v-for="body_ind in image_count/2" :key="body_ind">
              <div class="image-medium">
                <i class="el-icon-picture-outline" v-if="pushFHImages.body[image_count + body_ind-1].src == ''"></i>
                <img class="image-medium" v-if="pushFHImages.body[image_count + body_ind-1].src != ''" :src="pushFHImages.body[image_count + body_ind-1].src" @click="look(pushFHImages.body[image_count + body_ind-1])"></img>
              </div>
            </el-col>
          </el-col>
          <el-col class="fp_img_col" :span="11">
            <el-col class="fp_img_col" :span="3"  v-for="body_ind in image_count/2" :key="body_ind">
              <div class="image-medium">
                <i class="el-icon-picture-outline" v-if="pushFHImages.body[image_count/2 + image_count + body_ind-1].src == ''"></i>
                <img class="image-medium" v-if="pushFHImages.body[image_count/2 + image_count + body_ind-1].src != ''" :src="pushFHImages.body[image_count/2 + image_count + body_ind-1].src" @click="look(pushFHImages.body[image_count/2 + image_count + body_ind-1])"></img>
              </div>
            </el-col>
          </el-col>
        </el-row>
      </el-footer>
      <div class="view_container" v-show="showViewer">
        <el-dialog :visible.sync="showViewer" width="800px" :before-close="handleViewerClose">
          <span slot="title" class="dialog-title">
            {{image_types[viewerImg.type]+'详细信息-' + viewerImg.date}}
          </span>
          <el-row class="view_image_info_container">
            <el-col :span="12" class="view_image_col">
              <el-image class="image-view" :src="viewerImg.src" :preview-src-list="[viewerImg.src]" fit="fill">
                <div slot="error" class="image-slot-large">
                  <i class="el-icon-picture-outline"></i>
                </div>
              </el-image>
            </el-col>
            <el-col :span="12" class="info_col">
              <div class="view_info_container">
                <div class="view_info_text" v-show="viewerImg.type == 3 && detect_mode == 'facehuman'">
                  性别：    {{viewerImg.info.gender}}
                </div>
                <div class="view_info_text"  v-show="viewerImg.type == 3 && detect_mode == 'facehuman'">
                  年龄：    {{viewerImg.info.age}}
                </div>
                <div class="view_info_text"  v-show="viewerImg.type == 3 && detect_mode == 'facehuman'">
                  口罩：    {{viewerImg.info.mask}}
                </div>
                <div class="view_info_text"  v-show="viewerImg.type == 3 && detect_mode == 'facehuman'">
                  识别：    {{viewerImg.info.info}}
                </div>
                <div class="view_info_text"  v-show="viewerImg.type == 4 && detect_mode == 'hvcfp'">
                  车牌号码：{{viewerImg.info.number}}
                </div>
                <div class="view_info_text"  v-show="viewerImg.type == 4 && detect_mode == 'hvcfp'">
                  车牌颜色：{{viewerImg.info.color}}
                </div>
                <div class="view_info_text"  v-show="detect_mode == 'facehuman' && viewerImg.type != 3 || detect_mode == 'hvcfp' && viewerImg.type !=4">
                  无属性信息
                </div>
              </div>
            </el-col>
          </el-row>
          <!--
          <span slot="footer" class="dialog-footer">
            <el-button type="primary" @click="showViewer = false">确 定</el-button>
          </span>
          -->
        </el-dialog>
      </div>
    </el-container>
  </div>
</template>

<script>
import Wfs from '../plugins/wfs-min.js'
import screenfull from 'screenfull'
import { saveAs } from "file-saver"
export default {
  props:["mode_option"],
  data () {
    return {
      image_count: 16,
      detect_mode: 'hvcfp',
      //detect_mode: 'facehuman',
      pushFHImages: {
        face: [],
        body: []
      },
      pushHCVFPImages: {
        face: [],
        body: [],
        vechicle: [],
        cycle: [],
        plate: []
      },
      currentImg: {
        type: 0,
        src: '',
        date: '',
        info: {
          age: '--',
          gender: '--',
          mask: '--',
          info: '--',
          number: '--',
          color: '--'
        }
      },
      viewerImg: {
        type: 0,
        src: '',
        date: '',
        info: {
          age: '--',
          gender: '--',
          mask: '--',
          info: '--',
          number: '--',
          color: '--'
        }
      },
      eventList: [
      ],
      image_types: ['人形', '机动车', '非机动车', '人脸', '车牌'],
      sizeOptions: [
        {
          id: 0,
          label: '自适应',
          value: '0:0',
          icon: 'el-icon-full-screen'
        },
        {
          id: 1,
          label: '原比例',
          value: '1:1',
          icon: 'el-icon-full-screen'
        }
      ],
      srcOptions: [
        {
          id: 0,
          label: '视频源0',
          icon: 'el-icon-full-screen'
        },
        {
          id: 1,
          label: '视频源1',
          icon: 'el-icon-full-screen'
        }
      ],
      streamOptions: [
        {
          id: 0,
          label: '主码流0',
          icon: 'el-icon-full-screen'
        },
        {
          id: 1,
          label: '子码流1',
          icon: 'el-icon-full-screen'
        }
      ],
      genderOptions: [ '女', '男'],
      respiratorOptions: {'no_respirator':'未戴口罩',
                           'surgical':'医用口罩',
                           'anti-pollution':'防雾霾口罩',
                           'common':'普通口罩',
                           'kitchen_transparent':'厨房透明口罩',
                           'unknown':'未知'},
      plateColorOptions: {'blue':'蓝色',
                           'yellow':'黄色',
                           'black':'黑色',
                           'white':'白色',
                           'green':'绿色',
                           'small_new_energy':'新能源',
                           'large_new_energy':'新能源',
                           'absence':'未知',
                           'unknown':'未知'},
      screenIcon: 'vjs-icon-fullscreen-enter',
      sizeOptionInd: 1,
      srcOptionInd: 0,
      streamOptionInd: 1,
      rec_enable: true,
      isAiEnable: false,
      isSearchImg: false,
      isFullScreen: false,
      isRecStart: false,
      isError: false,
      wfsObj: null,
      isForceStop: false,
      wsCapture: [null, null],
      wsEvents: null,
      timer_capture: [undefined, undefined],
      timerEvents: undefined,
      lastRecvDataTime: [0, 0],
      curImgInd: {
        fh: [0, 0],  // face, body
        hcvfp: [0, 0, 0, 0, 0] // face, body, vechicle, cycle plate
      },
      max_try_num: 3,
      enc_info: '',
      timer_enc_info: undefined,
      showViewer: false,
      timerTest: null,
      curInd: 0,
      curListInd: 0,
    }
  },
  created () {
    /*
    var jpgDataBf = new ArrayBuffer(200)
    var jpgData = new Uint8Array(jpgDataBf)
    jpgData[0] = 65
    jpgData[1] = 88
    jpgData[2] = 73
    jpgData[3] = 84
    jpgData[4] = 26
    jpgData[5] = 0
    jpgData[6] = 0
    jpgData[7] = 0
    jpgData[8] = 74
    jpgData[9] = 83
    jpgData[10] = 79
    jpgData[11] = 78
    jpgData[12] = 11
    jpgData[13] = 0
    jpgData[14] = 0
    jpgData[15] = 0

    jpgData[16] = 0x7B
    jpgData[17] = 0x22
    jpgData[18] = 0x74
    jpgData[19] = 0x79
    jpgData[20] = 0x70
    jpgData[21] = 0x65
    jpgData[22] = 0x22
    jpgData[23] = 0x3A
    jpgData[24] = 0x31
    jpgData[25] = 0x7D
    jpgData[26] = 0

    this.parseJpgDataWithHead(jpgData)
    */
    console.log('preview created ++')
    this.getInfo()

    var ind = window.localStorage.getItem('stream')
    if (ind === '0') {
      this.streamOptionInd = 0
    } else if (ind === '1') {
      this.streamOptionInd = 1
    } else if (ind === '2') {
      this.streamOptionInd = 2
    } else {
      this.streamOptionInd = 1
    }

    ind = window.localStorage.getItem('src_id')
    if (ind === '1') {
      this.srcOptionInd = 1
    } else {
      this.srcOptionInd = 0
    }

    ind = window.localStorage.getItem('size')
    if (ind === '1') {
      this.sizeOptionInd = 1
    } else {
      this.sizeOptionInd = 0
    }

    for (var i = 0;i < this.image_count;i++) {
      this.pushFHImages.face.push({type:3, src: '', date: '', info: {gender: '--', age: '--', mask: '--', info: '--'}})
      this.pushFHImages.face.push({type:3, src: '', date: '', info: {gender: '--', age: '--', mask: '--', info: '--'}})
      this.pushFHImages.body.push({type:0, src: '', date: '', info: {}})
      this.pushFHImages.body.push({type:0,src: '', date: '', info: {}})

      this.pushHCVFPImages.face.push({type:3, src: '', date: '', info: {}})
      this.pushHCVFPImages.body.push({type:0, src: '', date: '', info: {}})
      this.pushHCVFPImages.vechicle.push({type:1, src: '', date: '', info: {}})
      this.pushHCVFPImages.cycle.push({type:2, src: '', date: '', info: {}})
      this.pushHCVFPImages.plate.push({type:4, src: '', date: '', info: {number: '--', color: '--'}})
    }

    /*
    // for test
    for (var i = 0;i < 32;i++) {
      this.eventList.push({type:'遮挡', date: '19:20:10'})
    }

    this.timerTest = setInterval(() => {
        var typeImg =  (this.curListInd + 1) % 5
        if (typeImg === 0) {
          this.pushFHImages.body[this.curListInd].src = this.imgTestList[this.curInd]
          this.pushFHImages.body[this.curListInd].date = '2020-03-24 10:20:59'
        } else if (typeImg === 1) {
          this.pushHCVFPImages.vechicle[this.curListInd].src = this.imgTestList[this.curInd]
          this.pushHCVFPImages.vechicle[this.curListInd].date = '2020-03-24 10:20:59'
        } else if (typeImg === 2) {
          this.pushHCVFPImages.cycle[this.curListInd].src = this.imgTestList[this.curInd]
          this.pushHCVFPImages.cycle[this.curListInd].date = '2020-03-24 10:20:59'
        } else if (typeImg === 3) {
          this.pushFHImages.face[this.curListInd].src = this.imgTestList[this.curInd]
          this.pushFHImages.face[this.curListInd].date = '2020-03-24 10:20:59'
          this.pushFHImages.face[this.curListInd].info.gender = this.genderOptions[1]
          this.pushFHImages.face[this.curListInd].info.age = 18
          this.pushFHImages.face[this.curListInd].info.mask = 'aaaaaaaaaaaaaaqqqqqqqqqqqqqqqqqqqqqqqqq'
          this.pushFHImages.face[this.curListInd].info.info = 'bbbbbbbbbbbbbdddddddddddddddddd'
        } else if (typeImg === 4) {
          this.pushHCVFPImages.plate[this.curListInd].src = this.imgTestList[this.curInd]
          this.pushHCVFPImages.plate[this.curListInd].date = '2020-03-24 10:20:59'
          this.pushHCVFPImages.plate[this.curListInd].info.number = '沪AAG2683'
          this.pushHCVFPImages.plate[this.curListInd].info.color = 'small_new_energy'
          //this.currentImg = this.pushHCVFPImages.plate[this.curListInd]
        }
        this.curInd = (this.curInd + 1) % 5
        this.curListInd = (this.curListInd + 1) % this.image_count

      }, 500)
      */
    if (!document.getElementById('converterScript')) {
      let script = document.createElement('script')
      script.setAttribute('src', 'js/wasmconverter.js')
      script.setAttribute('type', 'text/javascript')
      script.setAttribute('id', 'converterScript');
      document.head.appendChild(script);
    }

    console.log('preview created --')
  },
  mounted () {
    console.log('preview mounted ++')
    const _this = this
    if (screenfull.isEnabled) {
      screenfull.on('change', () => {
        _this.checkFull()
      })
    }

    this.startMedia()
    console.log('preview mounted --')
  },
  destroyed () {
    this.stopMedia()
  },
  methods: {
    wasmConvertImage(data) {
      const jpgFormat = 0;
      const pngFormat = 1;
      // data: Uint8Array
      // informat/outformat: 0:JPEG, 1:PNG
      var ret = Module.convertImage(data, data.length, jpgFormat, pngFormat);
      return ret;
    },
    startMedia () {
      this.startPlay()
      this.startCapture(0)
      this.startCapture(1)
      this.startRecvEvents()
    },
    stopMedia () {
      this.isForceStop = true
      this.stopEncInfoQuery()
      this.stop()
      this.stopCatpure(0)
      this.stopCatpure(1)
      this.stopRecvEvents()
      screenfull.off('change', () => {})
    },
    async startPlay () {
      var is_play_ok = false
      for (var i = 0; i < this.max_try_num; i++) {
        if (is_play_ok) {
          break
        }

        try {
          await this.changeStream(this.streamOptionInd + '', false, false)
          this.play()
          is_play_ok = true
        } catch (err) {
          is_play_ok = false
          if (i == this.max_try_num - 1) {
            console.log('startPlay catch except: ' + err)
          }
        }
      }

      if (!is_play_ok) {
        this.$message.error('网络请求码流失败')
      }
    },
    play () {
      console.log('play ++')
      try {
        this.stop()
        this.changeSizeOption(this.sizeOptionInd + '')

        const tokenStr = window.sessionStorage.getItem('token')
        const myVideo = document.getElementById('myVideo')

        let media = myVideo
        let mediaType = 'H264Raw'

        if (Wfs.isSupported()) {
          let config = {
            wsMinPacketInterval: 2000, // 最大接收数据超时时间，单位毫秒
            wsMaxPacketInterval: 8000  // 检查超时间隔，单位毫秒
          };
          this.wfsObj = new Wfs(config)
          this.wfsObj.attachMedia(media, 'ch1', mediaType, 'ws/preview?token=' + tokenStr)
          this.wfsObj.on(Wfs.Events.ERROR, (eventName, data) => this.handleWfsEvent(eventName, data))

          if (this.isRecStart) {
            this.isRecStart = false
            this.onRecording()
          }

          this.startEncInfoQuery()
        } else {
          console.log('wfs is not supported')
        }
      } catch (err) {
        console.log('play catch except: ' + err)
      }
      console.log('play --')
    },
    stop () {
      console.log('stop ++')
      if (this.wfsObj != null) {
        if (this.isRecStart) {
          this.wfsObj.stop_recording()
          if (!this.isError) {
            this.isRecStart = false
          }
        }
        this.wfsObj.destroy()
        this.wfsObj = null
      }
      console.log('stop --')
    },
    changeSizeOption (command) {
      this.sizeOptionInd = Number(command)
      if (this.sizeOptionInd === 0) {
        document.getElementById('myVideo').style.setProperty('object-fit', 'fill', 'important')
      } else {
        document.getElementById('myVideo').style.setProperty('object-fit', 'contain', 'important')
      }
      console.log('old objectFit: ', document.getElementById('myVideo').style.objectFit)
      window.localStorage.setItem('size', this.sizeOptionInd)
      // this.scaleVideoTo16vs9()
    },
    async changeStream (command, replay = true, changeData = true) {
      // request http to send this configure to server
      const steamInd = Number(command)
      console.log('change stream to ' + this.streamOptions[steamInd].id)

      const { data: res } = await this.$http.post('preview/stream', {
                                                                      src: this.srcOptions[this.srcOptionInd].id,
                                                                      stream: this.streamOptions[steamInd].id
                                                                    })
      console.log('post stream return: ', res)
      if (!res || res.meta.status !== 200) return this.$message.error('设置码流失败')
      if (changeData) {
        this.streamOptionInd = steamInd
        window.localStorage.setItem('src_id', this.srcOptions[this.srcOptionInd].id)
        window.localStorage.setItem('stream', this.streamOptions[steamInd].id)
      }
      if (replay) {
        this.play()
      }
      this.clearEncInfo()
    },

    async recordFaceinfo () {
      // request http to send this configure to server
      console.log('record faceinfo')
      this.stop()
          const { data: res } = await this.$http.post('preview/faceinfo', { faceinfo: true })
      this.play()
          console.log('post record faceinfo: ', res)
          if (res.meta.status !== 200) {
        return this.$message.error('人像信息录入失败')
      }
      else {
        if (res.data.faceinfo.status == 0) {
          return this.$message.success('人像信息录入成功')
        }
        else if (res.data.faceinfo.status == 2) {
          return this.$message.warning('不支持此功能')
        }
        else if (res.data.faceinfo.status == 3) {
          return this.$message.warning('数据加载中，请稍后再试')
        }
        else {
          return this.$message.error('人像信息录入失败，请重试')
        }
      }
    },
    async getInfo () {
      try {
        const { data: res } = await this.$http.get('preview/info')
        console.log('preview info get return: ', res)
        if (res.meta.status === 200) {
          this.isSearchImg = res.data.info.searchimg
          this.isAiEnable = res.data.info.ai_enable
          this.detect_mode = res.data.info.detect_mode
          console.log('detect_mode: ' + this.detect_mode)
          if (!this.detect_mode) {
            this.detect_mode = 'facehuman'
            console.log('set detect_mode to default: ' + this.detect_mode)
          }
        }
      } catch (error) {
        this.$message.error('获取信息失败')
      }
    },
    rec_result_cb(result) {
      this.rec_enable = true
      if (result) {
        this.isRecStart = true
      }
      else {
        this.$message.error('开启录像失败')
      }
    },
    rec_state_cb(state) {
      this.rec_enable = state
    },
    async onSnapshot() {
      try {
        const { data: res } = await this.$http.get('preview/snapshot')
        if (res.meta.status === 200) {
          console.log('send snapshot cmd succ')
        }
      } catch (error) {
        this.$message.error('发送命令失败')
      }
    },
    async onRecording() {
      if (this.wfsObj) {
        if (!this.isRecStart) {
          this.rec_enable = false
          var rec_threshold_web = window.localStorage.getItem('rec_threshold_web')
          if (rec_threshold_web == undefined) {
            rec_threshold_web = 1024
          }
          console.log('recording - threshold: ' + rec_threshold_web + ' MB')
          this.wfsObj.start_recording(rec_threshold_web*1024*1024, this.rec_result_cb, this.rec_state_cb)
        }
        else {
          this.wfsObj.stop_recording()
          this.isRecStart = false
        }
      }
    },
    async getEncInfo () {
      try {
        const { data: res } = await this.$http.get('preview/assist')
        console.log('preview enc info get return: ', res)
        if (res.meta.status === 200) {
          this.enc_info = '分辨率: ' + res.data.assist_res + '\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0' + '码率: ' + res.data.assist_bitrate
        }
      } catch (error) {
        this.$message.error('获取信息失败')
      }
    },
    async clearEncInfo () {
      this.enc_info = ''
    },
    startEncInfoQuery() {
      this.stopEncInfoQuery()
      this.timer_enc_info = setInterval(() => {
        this.getEncInfo()
      }, 1000)
    },
    stopEncInfoQuery() {
      if (this.timer_enc_info) {
        clearInterval(this.timer_enc_info)
        this.timer_enc_info = undefined
      }
    },
    doScreen () {
      const element = document.getElementsByClassName('preview_container')[0]
      if (this.isFullScreen) {
        screenfull.exit(element)
        this.screenIcon = 'vjs-icon-fullscreen-enter'
      } else {
        screenfull.request(element)
        this.screenIcon = 'vjs-icon-fullscreen-exit'
      }
      this.isFullScreen = !this.isFullScreen
    },
    checkFull () {
      if (!screenfull.isFullscreen) {
        this.isFullScreen = false
        this.screenIcon = 'vjs-icon-fullscreen-enter'
      }
    },
    scaleVideoTo16vs9 () {
      var myVideo = document.getElementById('myVideo')
      console.log('videoRatio:' + myVideo.videoWidth + '/' + myVideo.videoHeight)
      /*
      var videoRatio = myVideo.videoWidth / myVideo.videoHeight
      console.log('videoRatio:' + videoRatio)
      var tagRatio = myVideo.width() / myVideo.height()
      console.log('tagRatio:' + tagRatio)
      if (videoRatio < tagRatio) {
        myVideo.css('-webkit-transform', 'scaleX(' + tagRatio / videoRatio + ')')
      } else if (tagRatio < videoRatio) {
        myVideo.css('-webkit-transform', 'scaleY(' + videoRatio / tagRatio + ')')
      }
      */
    },
    handleWfsEvent (eventName, data) {
      try {
        if (data.fatal) {
          this.isError = true
          console.warn('Fatal error :' + data.details)
          switch (data.type) {
            case Wfs.ErrorTypes.MEDIA_ERROR:
              console.log('a media error occurred')
              break
            case Wfs.ErrorTypes.NETWORK_ERROR:
              console.log('a network error occurred')
              break
            default:
              console.log('an unrecoverable error occurred')
              break
          }
          this.play()
        }
      } catch (error) {
        console.log('handleWfsEvent catch except: ' + error.description)
      }
    },
    startCapture(i) {
      this.stopCatpure(i)
      const tokenStr = window.sessionStorage.getItem('token')
      var uri = 'ws://' + window.location.host + '/ws/capture_' + i + '?token=' + tokenStr
      var protocol = 'binary'
      this.wsCapture[i] = new WebSocket(uri, protocol)
      // var uri = 'ws://10.126.11.244:8080'
      // this.wsCapture = new WebSocket(uri)
      this.wsCapture[i].onopen = () => {
        console.log('ws_' + i + ' capture connected')
        this.create_capture_timer(i)
      }
      this.wsCapture[i].onclose = (err) => {
        console.log('ws_' + i + ' capture disconnected', err)
        if (!this.isForceStop) {
          this.startCapture(i)
        }
      }
      this.wsCapture[i].onmessage = (event) => {
        if (typeof (event.data) === 'string') {
          console.log('got a wscapture message: ' + event.data)
        } else {
          // console.log('got a wscapture image data, data-type:' + typeof (event.data) )
          try {
            this.lastRecvDataTime[i] = performance.now()
            if (i === 0) {
              var reader = new FileReader()
              reader.onload = (evt) => {
                if (evt.target.readyState === FileReader.DONE) {
                  this.parseJpgDataWithHead(evt.target.result)
                }
              }
              reader.readAsArrayBuffer(event.data)
            }
            else if (i === 1) {
              const blob = new Blob([event.data], { type: 'image/jpeg' })
              const fileName = 'snapshot_' + this.getCurrentTime() +'.jpg'
              saveAs(blob, fileName)
            }
          } catch (err) {
            console.log('capture image catch except: ' + err)
          }
        }
      }
      this.wsCapture[i].onerror = (err) => {
        console.log('ws_' + i + ' capture occure error', err)
      }
    },
    stopCatpure(i) {
      this.clear_capture_timer(i)
      if (this.wsCapture[i]) {
        this.wsCapture[i].close()
      }
    },
    create_capture_timer(i) {
      this.clear_capture_timer(i)
      this.timer_capture[i] = setInterval(() => {
        if (this.wsCapture[i]) {
          this.wsCapture[i].send("keep-alive")
        }
      }, 3000)
    },
    clear_capture_timer(i) {
      if (this.timer_capture[i]) {
        clearInterval(this.timer_capture[i])
        this.timer_capture[i] = undefined
      }
    },
    startRecvEvents () {
      console.log('startRecvEvents ++')
      this.stopRecvEvents()
      const tokenStr = window.sessionStorage.getItem('token')
      var uri = 'ws://' + window.location.host + '/ws/events?token=' + tokenStr
      var protocol = 'binary'
      this.wsEvents = new WebSocket(uri, protocol)
      this.wsEvents.onopen = () => {
        console.log('ws events connected')
        this.createEventstimer()
      }
      this.wsEvents.onclose = (err) => {
        console.log('ws events disconnected', err)
        if (!this.isForceStop) {
          this.startRecvEvents()
        }
      }
      this.wsEvents.onmessage = (event) => {
        if (typeof (event.data) === 'string') {
          console.log('got a ws events message: ' + event.data)
        } else {
          try {
            var reader = new FileReader()
            reader.onload = (evt) => {
              if (evt.target.readyState === FileReader.DONE) {
                console.log('got a event: ' + evt.target.result)
                var eventObj = JSON.parse(evt.target.result)
                for (var i = 0;i < eventObj.events.length;i++) {
                  if (eventObj.events[i].type === 0) {
                    this.eventList.unshift({type: '移动', date: eventObj.events[i].date})
                  } else  if (eventObj.events[i].type === 1) {
                    this.eventList.unshift({type: '遮挡', date: eventObj.events[i].date})
                  }
                }

                if (this.eventList.length > 32) {
                  this.eventList.splice(32, this.eventList.length - 32)
                }
              }
            }
            reader.readAsText(event.data)
          } catch (err) {
            console.log('ws events catch except: ' + err)
          }
        }
      }
      this.wsEvents.onerror = (err) => {
        console.log('ws events occure error', err)
      }
      console.log('startRecvEvents --')
    },

    stopRecvEvents() {
      console.log('stopRecvEvents ++')
      this.clearEventstimer()
      if (this.wsEvents) {
        this.wsEvents.close()
      }
      console.log('stopRecvEvents --')
    },
    createEventstimer() {
      this.clearEventstimer()
      this.timerEvents = setInterval(() => {
        if (this.wsEvents) {
          this.wsEvents.send("keep-alive")
        }
      }, 3000)
    },
    clearEventstimer() {
      if (this.timerEvents) {
        clearInterval(this.timerEvents)
        this.timerEvents = undefined
      }
    },

    parseJpgDataWithHead(jpgData) {
      try {
        var jpgHdr = new Uint8Array(jpgData.slice(0,16))
        // console.log('-->jpg header: ' + jpgHdr)

        if (jpgHdr && jpgHdr[0] === 0x41 && jpgHdr[1] === 0x58 && jpgHdr[2] === 0x49 && jpgHdr[3] === 0x54) {
          var hdrLen = this.bytesToIntLittleEndian(jpgHdr.subarray(4,8))
          // console.log('ws parse jpg header: total-len=' + hdrLen)
          if (jpgHdr[8] === 0x4A && jpgHdr[9] === 0x53 && jpgHdr[10] === 0x4F && jpgHdr[11] === 0x4E) {
            var jsonLen = this.bytesToIntLittleEndian(jpgHdr.subarray(12,16))
            // console.log('ws parse jpg header: json-len=' + jsonLen)
            var enc = new TextDecoder("utf-8");
            var jsonData = new Uint8Array(jpgData.slice(16, 16+jsonLen-1))
            var jsonStr = enc.decode(jsonData)
            console.log('jpg info:' + jsonStr)
            var jpgInfo = JSON.parse(jsonStr)
            //console.log('jpg info - type:' + jpgInfo.type)

            var jpg = jpgData.slice(hdrLen)
            var png = this.wasmConvertImage(new Uint8Array(jpg));
            var img64 = btoa(new Uint8Array(png).reduce((data, byte) => data + String.fromCharCode(byte),''))
            var url = "data:image/png;base64," + img64

            // console.log('jpg url:' + url)

            if (jpgInfo && url)
            {
              // console.log('fill image info +++: ' + this.detect_mode)

              //JPEG_TYPE_BODY = 0,
              //JPEG_TYPE_VEHICLE,
              //JPEG_TYPE_CYCLE,
              //JPEG_TYPE_FACE,
              // JPEG_TYPE_PLATE,

              if (this.detect_mode === 'facehuman') {

                if (jpgInfo.type === 3) { // face
                  var index = this.curImgInd.fh[0]
                  this.pushFHImages.face[index].src = url
                  this.pushFHImages.face[index].date = this.getCurrentTime()
                  this.pushFHImages.face[index].info.gender = this.genderOptions[jpgInfo.attribute.face.gender]
                  this.pushFHImages.face[index].info.age = jpgInfo.attribute.face.age
                  this.pushFHImages.face[index].info.mask = this.respiratorOptions[jpgInfo.attribute.face.mask]
                  this.pushFHImages.face[index].info.info = jpgInfo.attribute.face.info
                  this.curImgInd.fh[0] = (index + 1) % this.pushFHImages.face.length

                  // this.currentImg.src = url + '?' + new Date().getTime()
                  // this.currentImg.src = url + '?t=' + Math.random()
                  this.currentImg.src = url
                  this.currentImg.type = 3
                  this.currentImg.date = this.pushFHImages.face[index].date
                  this.currentImg.info.gender = this.pushFHImages.face[index].info.gender
                  this.currentImg.info.age = this.pushFHImages.face[index].info.age
                  this.currentImg.info.mask = this.pushFHImages.face[index].info.mask
                  this.currentImg.info.info = this.pushFHImages.face[index].info.info

                } else if  (jpgInfo.type === 0) { // body
                  var index = this.curImgInd.fh[1]
                  this.pushFHImages.body[index].src = url
                  this.pushFHImages.body[index].date = this.getCurrentTime()
                  this.pushFHImages.face[index].info = {}
                  this.curImgInd.fh[1] = (index + 1) % this.pushFHImages.body.length
                }
              } else {
                if (jpgInfo.type === 3) { // face
                  var index = this.curImgInd.hcvfp[0]
                  this.pushHCVFPImages.face[index].src = url
                  this.pushHCVFPImages.face[index].date = this.getCurrentTime()
                  this.pushHCVFPImages.face[index].info = {}
                  this.curImgInd.hcvfp[0] = (index + 1) % this.pushHCVFPImages.face.length
                } else if  (jpgInfo.type === 0) { // body
                  var index = this.curImgInd.hcvfp[1]
                  this.pushHCVFPImages.body[index].src = url
                  this.pushHCVFPImages.body[index].date = this.getCurrentTime()
                  this.pushHCVFPImages.body[index].info = {}
                  this.curImgInd.hcvfp[1] = (index + 1) % this.pushHCVFPImages.body.length
                } else if  (jpgInfo.type === 1) { // vechicle
                  var index = this.curImgInd.hcvfp[2]
                  this.pushHCVFPImages.vechicle[index].src = url
                  this.pushHCVFPImages.vechicle[index].date = this.getCurrentTime()
                  this.pushHCVFPImages.vechicle[index].info = {}
                  this.curImgInd.hcvfp[2] = (index + 1) % this.pushHCVFPImages.vechicle.length
                } else if  (jpgInfo.type === 2) {
                  var index = this.curImgInd.hcvfp[3]
                  this.pushHCVFPImages.cycle[index].src = url
                  this.pushHCVFPImages.cycle[index].date = this.getCurrentTime()
                  this.pushHCVFPImages.cycle[index].info = {}
                  this.curImgInd.hcvfp[3] = (index + 1) % this.pushHCVFPImages.cycle.length
                } else if  (jpgInfo.type === 4) { // plate
                  var index = this.curImgInd.hcvfp[4]
                  this.pushHCVFPImages.plate[index].src = url
                  this.pushHCVFPImages.plate[index].date = this.getCurrentTime()
                  this.pushHCVFPImages.plate[index].info.number = jpgInfo.attribute.plate.num
                  this.pushHCVFPImages.plate[index].info.color = this.plateColorOptions[jpgInfo.attribute.plate.color]
                  this.curImgInd.hcvfp[4] = (index + 1) % this.pushHCVFPImages.plate.length

                  this.currentImg.src = url
                  this.currentImg.type = 4
                  this.currentImg.date = this.pushHCVFPImages.plate[index].date
                  this.currentImg.info.number = this.pushHCVFPImages.plate[index].info.number
                  this.currentImg.info.color = this.pushHCVFPImages.plate[index].info.color
                }
              }
            }
          }
        }
      } catch (err) {
        console.log('ws parse jpg with header catch except: ' + err)
      }
    },

    getCurrentTime () {
      var date = new Date()
      var month = this.zeroFill(date.getMonth() + 1)
      var day = this.zeroFill(date.getDate())
      var hour = this.zeroFill(date.getHours())
      var minute = this.zeroFill(date.getMinutes())
      var second = this.zeroFill(date.getSeconds())
      var ms = date.getMilliseconds()
      if (ms <= 9) {
        ms = '00' + ms
      } else if (ms <= 99) {
        ms = '0' + ms
      }
      var curTime = date.getFullYear() + '-' + month + '-' + day + ' ' + hour + ':' + minute + ':' + second + '.' + ms
      return curTime
    },
    zeroFill (i) {
      if (i >= 0 && i <= 9) {
        return '0' + i
      } else {
        return i
      }
    },
    bytesToIntLittleEndian (bytes) {
      var val = 0
      for (var i = bytes.length - 1; i >= 0; i--) {
        val += bytes[i]
        if (i != 0) {
          val = val << 8
        }
      }
      return val
    },
    handleViewerClose() {
      this.showViewer=false
    },
    look(imgInfo) {

      if ((this.detect_mode === 'facehuman' && imgInfo.type === 3) || (this.detect_mode === 'hvcfp' && imgInfo.type === 4)) {
        this.currentImg.type = imgInfo.type
        this.currentImg.src = imgInfo.src
        this.currentImg.date = imgInfo.date
        this.currentImg.info.age = imgInfo.info.age
        this.currentImg.info.gender = imgInfo.info.gender
        this.currentImg.info.mask = imgInfo.info.mask
        this.currentImg.info.info = imgInfo.info.info
        this.currentImg.info.number = imgInfo.info.number
        this.currentImg.info.color = imgInfo.info.color
      }

      this.viewerImg.type = imgInfo.type
      this.viewerImg.src = imgInfo.src
      this.viewerImg.date = imgInfo.date
      this.viewerImg.info.age = imgInfo.info.age
      this.viewerImg.info.gender = imgInfo.info.gender
      this.viewerImg.info.mask = imgInfo.info.mask
      this.viewerImg.info.info = imgInfo.info.info
      this.viewerImg.info.number = imgInfo.info.number
      this.viewerImg.info.color = imgInfo.info.color
      this.showViewer = true
    }
  }
}
</script>

<style lang="less" scoped>
.total_container {
  height: 100%!important;
}
.preview_outside_container_normal {
  height: 100%!important;
}
.preview_outside_container {
  height: calc(100vh - 274px);
}
.side_identify_events_container {
  padding: 0!important;
  margin: 0!important;
}
.preview_main_container{
  height: 100%!important;
}
.preview_capture_left_container {
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
  .video_controls_container {
    padding: 0!important;
    background-color: #4c4c4c;
    display: flex;
    justify-content: space-between;
    align-items: center;
    .left_controls_container {
      display: flex;
      justify-content: left;
      color: #fff;
      .size_option {
        width: 80px;
        margin-left: 10px;
      }
      .size_src {
        width: 80px;
      }
      .size_info {
        font-size: 10px;
        display: flex;
        align-items: center;
        margin-left: 30px;
      }
    }
    .right_controls_container {
      display: flex;
      justify-content: left;
      flex-direction: row;
      align-content: center;
      align-items: stretch;
    }
  }
}
.draw_button_enable {
  background-color: transparent!important;
  color: #fff;
  border: none;
  width: 28px;
  height: 28px;
  padding: 0;
  margin-left: 10px;
  /deep/ i {
    font-size: 20px;
  }
}
.draw_button_disable {
  background-color: transparent!important;
  color: #5c5c5c;
  border: none;
  width: 28px;
  height: 28px;
  padding: 0;
  margin-left: 10px;
  /deep/ i {
    font-size: 20px;
  }
}
.full_button {
  background-color: transparent!important;
  color: #fff;
  border: none;
  width: 28px;
  height: 28px;
  padding: 0;
  margin-left: 10px;
  /deep/ i {
    font-size: 26px;
  }
}
.el-dropdown-link {
  color: #fff;
}
.el-dropdown-menu {
  width: 120px;
}
.hvcfp_img_container {
  height: 210px!important;
  margin-left: 0px!important;
  padding-left: 0px!important;
  margin-right: 0px!important;
  padding-right: 0px!important;
}

.fh_img_container {
  height: 210px!important;
  margin-left: 0px!important;
  padding-left: 0px!important;
  margin-right: 0px!important;
  padding-right: 0px!important;
}
.img_list_top {
  margin-top: 10px!important;
}

.img_list_bottom {
  margin-bottom: 0px!important;
}
.img_list {
  margin: 0px;
  padding: 0px;
}
.capture_img_container {
  height: 100%;
}
.happy_scroll_container {
  height: 100%;
}
.el-col {
  background: #4c4c4c;
}

.fp_img_col {
  height: 50px!important;
}
.hvcfp_img_col {
    height: 40px!important;
}
.image-small{
  height: 40px;
  width: 40px;
  position: relative;
  display: inline-block;
  overflow: hidden;
  justify-content: center;
  align-items: center;
  background: #4c4c4c;
}

.image-medium{
  height: 50px;
  width: 50px;
  position: relative;
  display: inline-block;
  overflow: hidden;
  justify-content: center;
  align-items: center;
  background: #4c4c4c;
}

.image-large {
  height: 160px;
  width: 160px;
  position: relative;
  display: inline-block;
  overflow: hidden;
  justify-content: center;
  align-items: center;
  background: #4c4c4c;
}

.image-slot-large {
  display: flex;
  justify-content: center;
  align-items: center;
  width: 100%;
  height: 100%;
  background: #4c4c4c;
  color: #909399;
}
.el-icon-picture-outline {
  position: absolute; left: 50%; top: 50%;
  transform: translate(-50%, -50%);
  color: gray;
  font-size: 30px;
}

.identify_events_container {
  width: 100%!important;
  height: 100%!important;
}
.identify_container {
  height: 160px!important;
  padding-top: 0!important;
  padding-right: 0!important;
  padding-left: 10px!important;
  white-space:nowrap;
  .image_info_container {
    background-color: #4c4c4c;
    height: 160px!important;
    display:flex;
    // justify-content:center;/*主轴上居中*/
    align-items:center;/*侧轴上居中*/
    .image_col {
      height: 160px!important;
      width: 160px!important;
      margin: 0;
      padding: 0;
    }
    .info_container {
      padding-left: 10px;
      .info_text {
        color: #FFFFFF;
        font-size: 12px;
        margin-bottom: 10px;
      }
    }

  }
}
.view_container {
  background: #FFFFFF!important;
  height: 600px!important;
  padding-top: 0!important;
  padding-right: 0!important;
  padding-left: 10px!important;

  .dialog-title {
    font-size: 24px;
  }
  .view_image_info_container {
    background-color: #FFFFFF!important;
    height: 300px!important;

    .view_image_col {
      height: 300px!important;
      width: 300px!important;
      .image-view {
        height: 300px!important;
        width: 300px!important;
        background-color: #FFFFFF!important;
      }
    }
    .view_info_container {
      margin:0; padding:0; height:100%;
      background-color: #FFFFFF!important;
      padding-left: 40px;
      .view_info_text {
        color: #000000;
        font-size: 18px;
        padding-bottom: 18px;
        background-color: #FFFFFF!important;
      }
    }
  }
}
.type_text {
  display: flex;
  justify-content: center;
  align-items: center;
  height: 40px;
  width: 80px;
  background-color: #4c4c4c;
  color: #FFFFFF;
  font-size: 12px;
}

.type_text_fh {
  display: flex;
  justify-content: center;
  align-items: center;
  height: 50px;
  width: 80px;
  background-color: #4c4c4c;
  color: #FFFFFF;
  font-size: 12px;
}
.events_container {
  width: 100%!important;
  height: 100%!important;
  padding-left: 10px!important;
  padding-top: 0!important;
  padding-right: 0!important;
  padding-bottom: 0!important;
  margin-top: 0!important;
  margin-right: 0!important;
  margin-bottom: 0!important;
}
.events_table {
  width: 100%!important;
  height: 100%!important;
  padding-top: 0!important;
  padding-right: 0!important;
  padding-bottom: 0!important;
  margin-top: 0!important;
  margin-right: 0!important;
  margin-bottom: 0!important;
  background-color: #4c4c4c!important;
  border: none;
}
/deep/ .el-table tr {
  background-color: transparent !important;
  color: #E0E0E0;
}
/deep/ .el-table--enable-row-transition .el-table__body td,
  /deep/.el-table .cell {
    background-color: transparent;
    color: rgba(255,255,255,0.7);
    border: none;
  }
/deep/ .el-table__body tr.current-row>td {
  background-color: #5C5C5C;;
}

.el-table::before {
  //去除底部白线
  left: 0;
  bottom: 0;
  width: 100%;
  height: 0px;
}
/deep/ .el-table--enable-row-hover {
  .el-table__body tr:hover > td {
    background: #333333;
  }
}

</style>
