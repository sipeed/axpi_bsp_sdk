import Vue from 'vue'
import { Button, Form, FormItem, Checkbox, Input, Divider, Message, Container, Header, Aside, Main, Menu, MenuItem, Dropdown, DropdownMenu, DropdownItem, RadioButton, Tooltip, Switch, TimeSelect, InputNumber, ColorPicker, Select, Option, Footer, Row, Col, Image, Tabs, TabPane, Slider, Table, TableColumn, Dialog} from 'element-ui'

/* config Message options */
Vue.prototype.$message = function (msg) {
  Message(msg)
}
Vue.prototype.$message = function (msg) {
  return Message({
    message:msg,
    duration:2000
  })
}
Vue.prototype.$message.success = function (msg) {
  return Message.success({
    message: msg,
    duration: 1000
  })
}
Vue.prototype.$message.warning = function (msg) {
  return Message.warning({
    message: msg,
    duration: 1500
  })
}
Vue.prototype.$message.error = function (msg) {
  return Message.error({
    message: msg,
    duration: 3000
  })
}

Vue.use(Button)
Vue.use(Form)
Vue.use(FormItem)
Vue.use(Checkbox)
Vue.use(Input)
Vue.use(Divider)
Vue.use(Container)
Vue.use(Header)
Vue.use(Aside)
Vue.use(Main)
Vue.use(Menu)
Vue.use(MenuItem)
Vue.use(Dropdown)
Vue.use(DropdownMenu)
Vue.use(DropdownItem)
Vue.use(RadioButton)
Vue.use(Tooltip)
Vue.use(Switch)
Vue.use(TimeSelect)
Vue.use(InputNumber)
Vue.use(ColorPicker)
Vue.use(Select)
Vue.use(Option)
Vue.use(Footer)
Vue.use(Row)
Vue.use(Col)
Vue.use(Image)
Vue.use(Tabs)
Vue.use(TabPane)
Vue.use(Slider)
Vue.use(Table)
Vue.use(TableColumn)
Vue.use(Dialog)
