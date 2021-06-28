<script>
    export let name;
    export let min;
    export let max;
    export let resMin;
    export let resMax;

    if (!resMin) resMin = 0;
    if (!resMax) resMax = resMin;
</script>

<div class='holder'>
    <div class='name'>{name}</div>
    <div>
        <input type=number bind:value={resMin} min={min || undefined} max={resMax}> → <input type=number bind:value={resMax} min={resMin} max={max || undefined}>
    </div>
</div>

<style>
    input {
        width: 5.45em;
    }
</style>