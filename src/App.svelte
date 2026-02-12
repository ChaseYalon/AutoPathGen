<script lang="ts">
    import Control from "./lib/Control.svelte";
    import Top from "./lib/Top.svelte";
    import Side from "./lib/side.svelte";
</script>

<div class="app-container">
    <div class="top-panel">
        <Top />
    </div>
    <div class="main-content">
        <div class="control-panel">
            <Control/>
        </div>
        <div class="side-panel">
            <Side />
        </div>
    </div>
</div>

<style>
    :global(body) {
        margin: 0;
        padding: 0;
        overflow: hidden;
    }

    .app-container {
        display: flex;
        flex-direction: column;
        height: 100vh;
        width: 100vw;
    }

    .top-panel {
        height: 2%;
        width: 100%;
        background-color: #f0f0f0; 
    }

    .main-content {
        display: flex;
        flex: 1; 
        height: 98%; 
    }

    .side-panel {
        width: 25%;
        height: 100%;
    }

    .control-panel {
        width: 75%;
        height: 100%;
        overflow: hidden; 
        display: flex;
        box-sizing: border-box;
        border-top: 0.4em solid black;
        border-right: 0.4em solid black;
    }
</style>