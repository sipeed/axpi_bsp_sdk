<template>
  <div id="app">
    <router-view/>
  </div>
</template>

<script>
export default {
  name: 'app',
  created () {
    var baseUrl = window.location.href
    baseUrl = baseUrl.substring(0, baseUrl.indexOf('8080/') + 5) + 'action'
    this.$http.defaults.baseURL = baseUrl
    console.log(baseUrl)
  }
}
</script>

<style>

</style>
