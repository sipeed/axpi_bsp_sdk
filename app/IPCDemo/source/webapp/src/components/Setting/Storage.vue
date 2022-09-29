<template>
  <div class="storage-container">
    <el-form ref="formStorageRef" :rules="formStorageRules" :model="formStorage" label-position="left" label-width="180px">
      <el-form-item label="【板端存储策略】">
      </el-form-item>
      <el-form-item label="启用板端存储" class="storage-item">
        <el-switch v-model="formStorage.storage.rec_actived"></el-switch>
      </el-form-item>
      <el-form-item label="启用循环覆盖" class="storage-item">
        <el-switch v-model="formStorage.storage.rec_loop_cover" :disabled="!formStorage.storage.rec_actived"></el-switch>
      </el-form-item>
      <el-form-item label="录像文件总空间(MB)" class="storage-item">
        <el-col :span="8">
          <el-input v-model="formStorage.storage.rec_total" :readonly="true" :disabled="!formStorage.storage.rec_actived"></el-input>
        </el-col>
      </el-form-item>
      <el-form-item label="单个录像文件阈值(MB)" class="storage-item" prop="storage.rec_threshold">
        <el-input-number v-model="formStorage.storage.rec_threshold" :min="16" :max="1024" :step="64" :disabled="!formStorage.storage.rec_actived" controls-position="right" placeholder="文件阈值"></el-input-number>
      </el-form-item>
      <el-form-item label="录像文件数上限(个)" class="storage-item" prop="storage.rec_max_count">
        <el-input-number v-model="formStorage.storage.rec_max_count" :min="1" :max="100" :step="10" :disabled="!formStorage.storage.rec_actived" controls-position="right" placeholder="文件数上限"></el-input-number>
      </el-form-item>
      <el-form-item label="存储路径" class="storage-item" prop="storage.rec_path">
        <el-col :span="8">
          <el-input v-model="formStorage.storage.rec_path" :readonly="true" placeholder="板端存储路径"></el-input>
        </el-col>
      </el-form-item>
      <p style="color:indianred;margin-left: 60px;">注：单个录像文件阈值(MB) x 录像文件数上限(个)，须小于录像文件总空间(MB)</p>
      <el-form-item label="【前端存储策略】">
      </el-form-item>
      <el-form-item label="单个录像文件阈值(MB)" class="storage-item" prop="rec_threshold_web">
        <el-input-number v-model="formStorage.rec_threshold_web" :min="16" :max="1024" :step="64" controls-position="right" placeholder="文件阈值"></el-input-number>
      </el-form-item>
      <p style="color:indianred;margin-left: 60px;">注：录像回放仅涉及前端录像，下载录像时固定下载到当前用户的Downloads目录下</p>
      <el-form-item>
        <el-button type="primary" @click="onSubmit">修改</el-button>
      </el-form-item>
    </el-form>
  </div>
</template>

<script>
export default {
  data () {
    const validate_rec_path = (rule, value, callback) => {
      if (value.length > 64) {
        callback(new Error())
      }
      else {
        var p = /^\/(\w+\/?)+$/;
        if(!p.test(value))
        {
          callback(new Error())
        }
      }
      callback()
    }
    return {
      formStorage: {
        storage: {
          rec_actived: true,
          rec_loop_cover: true,
          rec_total: 2560,
          rec_threshold: 256,
          rec_max_count: 10,
          rec_path: '/mnt/sdcard/'
        },
        rec_threshold_web: 1024,
      },
      formStorageRules: {
        'storage.rec_threshold': [{ type: 'integer', required: true, message: '请输入板端录像文件阈值', trigger: 'blur' }],
        'storage.rec_max_count': [{ type: 'integer', required: true, message: '请输入板端录像文件数上限', trigger: 'blur' }],
        'storage.rec_path': [{ type: 'string', required: true, message: '请输入板端存储路径', trigger: 'blur' },
                             { required: true, validator: validate_rec_path, message: "板端存储路径长度超过64位，或路径无效(要求Linux目录格式，如：\'/mnt/sdcard/\')", trigger: "blur"}],
        'rec_threshold_web': [{ type: 'integer', required: true, message: '请输入前端录像文件阈值', trigger: 'blur' }]
      }
    }
  },
  created () {
    console.log('storage++')
    this.getInfo()
  },
  methods: {
    async onSubmit () {
      try {
        let _fileSize = this.formStorage.storage.rec_threshold
        let _fileNum  = this.formStorage.storage.rec_max_count
        if (_fileSize * _fileNum > this.formStorage.storage.rec_total) {
          this.$message.error('单个录像文件阈值 x 录像文件数上限, 超过板端总录像空间！')
          return
        }

        const { data: res } = await this.$http.post('setting/storage', this.formStorage)
        console.log('storage set return: ', res)
        if (res.meta.status === 200) {
          this.$message.success('修改成功')
        } else if (res.meta.status === 404) {
          this.$message.error('板端存储路径创建失败')
        } else {
          this.$message.error('修改失败')
        }
        window.localStorage.setItem('rec_threshold_web', this.formStorage.rec_threshold_web);
      } catch (error) {
        this.$message.error('修改失败')
      }
    },
    async getInfo () {
      try {
        const { data: res } = await this.$http.get('setting/storage')
        console.log('storage get return: ', res)
        if (res.meta.status === 200) {
          this.formStorage.storage = res.data.storage
          if (this.formStorage.storage.rec_total === 0) {
            this.formStorage.storage.rec_total = 2560
          }
        }
        var _rec_threshold_web = window.localStorage.getItem('rec_threshold_web')
        if (_rec_threshold_web != undefined) {
          this.formStorage.rec_threshold_web = _rec_threshold_web
        }
      } catch (error) {
        this.$message.error('获取信息失败')
      }
    }
  }
}
</script>

<style lang="less" scoped>
.storage-container {
  height: 100%;
}
.storage-item {
  margin-left: 60px;
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
