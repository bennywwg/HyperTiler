<script>
    export let name;
    export let res;

    export let min;
    export let max;

    if (!res) res = 0;
</script>

<div class='holder'>
    <div class='name'>{name}</div>
    <div>
        <input type=number bind:value={res} min={min} max={max}>
    </div>
</div>

<style>
    .holder {
        display: flex;
        justify-content: space-between;
        width: 100%;
        margin-top: 0.2em;
        margin-bottom: 0.2em;
    }
</style>