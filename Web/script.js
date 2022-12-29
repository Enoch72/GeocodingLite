const georef_url = 'http://127.0.0.1:8081/';

new Autocomplete('#autocomplete', {

    // Search function can return a promise
    // which resolves with an array of results.
    // Here we set the path of the GeocodingLite search API

    search: input => {
        const url = `${georef_url}geocode?query=${encodeURI(input)}`;

        return new Promise(resolve => {
            if (input.length < 3) {
                return resolve([]);
            }

            fetch(url).
            then(response => response.json()).
            then(data => {
                resolve(data.results);
            });
        });
    },

    // Here we set the tex to display in the list box
    getResultValue: result => result.TEXT,

    // Here we set the action afer a value was selected 
    onSubmit: result => {
        alert('The coordinates of ' + result.NATION + ' ' + result.PLACE_NAME +
            ' are : ' + result.LATITUDE + '  ' + result.LONGITUDE);
    }
});