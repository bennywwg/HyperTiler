<script>
    import NumberInput from './NumberInput.svelte'
    import CheckboxInput from './CheckboxInput.svelte'

    export let previewOutputLevel;
    export let showInputTiles;
    export let showOutputTiles;
    export let showPixelRange;
    export let levelMin;
    export let levelMax;
</script>

<div>
    <NumberInput name='Preview Output Level' min={levelMin} max={levelMax} bind:res={previewOutputLevel}/>
    <CheckboxInput name='Show Pixel Range' bind:res={showPixelRange}/>
    <CheckboxInput name='Show Input Tiles' bind:res={showInputTiles}/>
    <CheckboxInput name='Show Output Tiles' bind:res={showOutputTiles}/>
</div>