import axios from 'axios';
//const todoBaseUrl = 'http://localhost:3000/api';
const todoBaseUrl = 'http://weathermood-14.us-west-2.elasticbeanstalk.com/';

export function listTodos(unaccomplishedOnly = false, searchText = '',start) {
    let url = `${todoBaseUrl}/todos`;
    let query = [];
    if (searchText)
        query.push(`searchText=${searchText}`);
    if (start)
        query.push(`start=${start}`);
    if(unaccomplishedOnly)
        query.push(`unaccomplishedOnly=${unaccomplishedOnly}`)
    if (query.length)
        url += '?' + query.join('&');
    
    console.log(`Making GET request to: ${url}`);
    
    return axios.get(url).then(function(res) {
        if (res.status !== 200)
            throw new Error(`Unexpected response code: ${res.status}`);

        return res.data;
    });
}


export function createTodo(mood, text) {
    let url = `${todoBaseUrl}/todos`;

    console.log(`Making POST request to: ${url}`);

    return axios.post(url, {
        mood,
        text
    }).then(function(res) {
        if (res.status !== 200)
            throw new Error(`Unexpected response code: ${res.status}`);

        return res.data;
    });
}


export function accomplishTodo(id) {
    let url = `${todoBaseUrl}/todos/${id}`;

    console.log(`Making POST request to: ${url}`);

    return axios.post(url).then(function(res) {
        if (res.status !== 200)
            throw new Error(`Unexpected response code: ${res.status}`);

        return res.data;
    });
}

