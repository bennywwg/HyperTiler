<script>
    import VectorInput from './VectorInput.svelte'

    export let name;
    export let res;
    export let defaultVal = undefined;
    
    if (!res) res = defaultVal ? defaultVal : { };
</script>

<div>
    {name}
    <div class='block'>
        <VectorInput name='min' bind:res={res.min}/>
        <VectorInput name='max' bind:res={res.max}/>
    </div>
</div>