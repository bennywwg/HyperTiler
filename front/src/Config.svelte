<script>
  import AABBInput from './AABBInput.svelte';
  import VectorInput from './VectorInput.svelte';
  import RangeInput from './RangeInput.svelte';
  import ImageEncoding from './ImageEncoding.svelte';
  import FormatInput from './FormatInput.svelte';

  export let res;

  if (!res) res = { };

  res.DatasetConfig = { };
  res.DatasetConfig.InputEncoding = {"BitDepth":16,"Encoding":0,"Gamma":1.0,"SwapEndian":true};
  res.DatasetConfig.OutputEncoding = {"BitDepth":16,"Encoding":0,"Gamma":1.0,"SwapEndian":false};

  res.SpatialConfig = { };
  res.SpatialConfig.OutputPixelOffset = [0, 0]; // defaulted, not implemented yet
</script>

<div>
    <FormatInput name='Input Format' bind:res={res.DatasetConfig.InputURIFormat} defaultVal={"https://spkit.org/datasets/remapped/{x3}_{y3}_{z3}.hgt"}/>
    <FormatInput name='Output Format' bind:res={res.DatasetConfig.OutputURIFormat} defaultVal={"./{x3}_{y3}_{z3}.png"}/>
    <AABBInput name='Output Pixel Range' bind:res={res.SpatialConfig.OutputPixelRange} defaultVal={{min:[0,1201*30], max:[1201*360,1201*120]}}/>
    <VectorInput name='Output Tile Size' bind:res={res.SpatialConfig.OutputTileSize} defaultVal={[512,512]} min=0/>
    <VectorInput name='Input Tile Size' bind:res={res.SpatialConfig.InputTileSize} defaultVal={[1201,1201]} min=0/>
    <VectorInput name='Output → Input Pixel Ratio' bind:res={res.SpatialConfig.OutputToInputPixelRatio}/>
    <RangeInput name='Output Level Range' bind:resMin={res.SpatialConfig.BeginOutputLevel} bind:resMax={res.SpatialConfig.EndOutputLevel} min=0 max=32/>
    <ImageEncoding name='Input Encoding'/>
</div>

<style>
    div {
        width: 100%;
    }
</style>