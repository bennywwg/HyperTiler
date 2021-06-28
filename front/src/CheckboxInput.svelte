<script>
    export let name;
    export let res;

    const getInputId = () => {
        return `id_${name}`
    }
</script>

<label for={getInputId()} class='holder'>
    <div class='name'>{name}</div>
    <input id={getInputId()} type=checkbox bind:checked={res}/>
</label>

<style>
    label, input {
        cursor: pointer;
    }
</style>