function argumentsToArray(args) {
    return [].concat.apply([], Array.prototype.slice.apply(args));
}

function flatten(v) {
    if (v.matrix == true) {
        v = transpose(v);
    }

    var n = v.length;
    var elemsAreArrays = false;

    if (Array.isArray(v[0])) {
        elemsAreArrays = true;
        n *= v[0].length;
    }

    var floats = new Float32Array(n);

    if (elemsAreArrays) {
        var idx = 0;
        for (var i = 0; i < v.length; ++i) {
            for (var j = 0; j < v[i].length; ++j) {
                floats[idx++] = v[i][j];
            }
        }
    }
    else {
        for (var i = 0; i < v.length; ++i) {
            floats[i] = v[i];
        }
    }

    return floats;
}

var sizeof = {
    'vec2': new Float32Array(flatten(vec2())).byteLength,
    'vec3': new Float32Array(flatten(vec3())).byteLength,
    'vec4': new Float32Array(flatten(vec4())).byteLength,
    'mat2': new Float32Array(flatten(mat2())).byteLength,
    'mat3': new Float32Array(flatten(mat3())).byteLength,
    'mat4': new Float32Array(flatten(mat4())).byteLength
};

function radians(degrees) {
    return degrees * Math.PI / 180.0;
}

// Vector Constructors
function vec2() {
    var vector = argumentsToArray(arguments);

    switch (vector.length) {
        case 0:
            vector.push(0.0);
        case 1:
            vector.push(0.0);
    }

    return vector.splice(0, 2);
}

function vec3() {
    var vector = argumentsToArray(arguments);

    switch (vector.length) {
        case 0:
            vector.push(0.0);
        case 1:
            vector.push(0.0);
        case 2:
            vector.push(0.0);
    }

    return vector.splice(0, 3);
}

function vec4() {
    var vector = argumentsToArray(arguments);

    switch (vector.length) {
        case 0:
            vector.push(0.0);
        case 1:
            vector.push(0.0);
        case 2:
            vector.push(0.0);
        case 3:
            vector.push(1.0);
    }

    return vector.splice(0, 4);
}

// Matrix Constructors
function mat2() {
    var vector = argumentsToArray(arguments);

    var matrix = [];
    switch (vector.length) {
        case 0:
            vector[0] = 1.0;
        case 1:
            matrix = [
                vec2(vector[0], 0.0),
                vec2(0.0, vector[0])
            ];
            break;
        default:
            matrix.push(vec2(vector));
            vector.splice(0, 2);
            matrix.push(vec2(vector));
            break;
    }

    matrix.matrix = true;
    return matrix;
}

function mat3() {
    var vector = argumentsToArray(arguments);

    var matrix = [];
    switch (vector.length) {
        case 0:
            vector[0] = 1.0;
        case 1:
            matrix = [
                vec3(vector[0], 0.0, 0.0),
                vec3(0.0, vector[0], 0.0),
                vec3(0.0, 0.0, vector[0])
            ];
            break;
        default:
            matrix.push(vec3(vector));
            vector.splice(0, 3);
            matrix.push(vec3(vector));
            vector.splice(0, 3);
            matrix.push(vec3(vector));
            break;
    }

    matrix.matrix = true;
    return matrix;
}

function mat4() {
    var vector = argumentsToArray(arguments);

    var matrix = [];
    switch (vector.length) {
        case 0:
            vector[0] = 1.0;
        case 1:
            matrix = [
                vec4(vector[0], 0.0, 0.0, 0.0),
                vec4(0.0, vector[0], 0.0, 0.0),
                vec4(0.0, 0.0, vector[0], 0.0),
                vec4(0.0, 0.0, 0.0, vector[0])
            ];
            break;
        default:
            matrix.push(vec4(vector));
            vector.splice(0, 4);
            matrix.push(vec4(vector));
            vector.splice(0, 4);
            matrix.push(vec4(vector));
            vector.splice(0, 4);
            matrix.push(vec4(vector));
            break;
    }

    matrix.matrix = true;
    return matrix;
}

// Generic Mathematical Operations for Vectors and Matrices
function add(u, v) {
    var result = [];

    if (u.matrix && v.matrix) {
        if (u.length != v.length) {
            throw "add(): trying to add matrices of different dimensions";
        }

        for (var i = 0; i < u.length; ++i) {
            if (u[i].length != v[i].length) {
                throw "add(): trying to add matrices of different dimensions";
            }
            result.push([]);

            for (var j = 0; j < u[i].length; ++j) {
                result[i].push(u[i][j] + v[i][j]);
            }
        }

        result.matrix = true;
        return result;
    } else if ((u.matrix && !v.matrix) || (!u.matrix && v.matrix)) {
        throw "add(): trying to add matrix and non-matrix variables";
    } else {
        if (u.length != v.length) {
            throw "add(): vectors are not the same dimension";
        }

        for (var i = 0; i < u.length; ++i) {
            result.push(u[i] + v[i]);
        }

        return result;
    }
}

function sub(u, v) {
    var result = [];

    if (u.matrix && v.matrix) {
        if (u.length != v.length) {
            throw "sub(): trying to subtract matrices of different dimensions";
        }

        for (var i = 0; i < u.length; ++i) {
            if (u[i].length != v[i].length) {
                throw "sub(): trying to subtact matrices of different dimensions";
            }
            result.push([]);

            for (var j = 0; j < u[i].length; ++j) {
                result[i].push(u[i][j] - v[i][j]);
            }
        }

        result.matrix = true;
        return result;
    } else if ((u.matrix && !v.matrix) || (!u.matrix && v.matrix)) {
        throw "sub(): trying to subtact matrix and non-matrix variables";
    } else {
        if (u.length != v.length) {
            throw "sub(): vectors are not the same length";
        }

        for (var i = 0; i < u.length; ++i) {
            result.push(u[i] - v[i]);
        }

        return result;
    }
}

function mult(u, v) {
    var result = [];

    if (u.matrix && v.matrix) {
        if (u.length != v.length) {
            throw "mult(): trying to add matrices of different dimensions";
        }

        for (var i = 0; i < u.length; ++i) {
            if (u[i].length != v[i].length) {
                throw "mult(): trying to add matrices of different dimensions";
            }
        }

        for (var i = 0; i < u.length; ++i) {
            result.push([]);

            for (var j = 0; j < v.length; ++j) {
                var sum = 0.0;
                for (var k = 0; k < u.length; ++k) {
                    sum += u[i][k] * v[k][j];
                }
                result[i].push(sum);
            }
        }

        result.matrix = true;

        return result;
    }
    
    if (u.matrix && (u.length == v.length)) {
        for (var i = 0; i < v.length; i++) {
            var sum = 0.0;
            for (var j = 0; j < v.length; j++) {
                sum += u[i][j] * v[j];
            }
            result.push(sum);
        }
        return result;
    } else {
        if (u.length != v.length) {
            throw "mult(): vectors are not the same dimension";
        }

        for (var i = 0; i < u.length; ++i) {
            result.push(u[i] * v[i]);
        }

        return result;
    }
}

function scale(u, s) {
    if (u.matrix) {
        for (var i = 0; i < u.length; ++i) {
            for (var j = 0; j < u[i].length; ++j) {
                u[i][j] *= s;
            }
        }
    } else {
        for (var i = 0; i < u.length; ++i) {
            u[i] *= s;
        }
    }

    return u;
}

function negate(u) {
    return scale(u, -1.0);
}

function equal(u, v) {
    if (u.length != v.length) { return false; }

    if (u.matrix && v.matrix) {
        for (var i = 0; i < u.length; ++i) {
            if (u[i].length != v[i].length) { return false; }
            for (var j = 0; j < u[i].length; ++j) {
                if (u[i][j] !== v[i][j]) { return false; }
            }
        }
    }
    else if (u.matrix && !v.matrix || !u.matrix && v.matrix) {
        return false;
    }
    else {
        for (var i = 0; i < u.length; ++i) {
            if (u[i] !== v[i]) { return false; }
        }
    }

    return true;
}

// Matrix-Specific Operations
function transpose(m) {
    var result = [];

    if (!m.matrix) {
        return "transpose(): trying to transpose a non-matrix";
    }

    for (var i = 0; i < m.length; ++i) {
        result.push([]);

        for (var j = 0; j < m[i].length; ++j) {
            result[i].push(m[j][i]);
        }
    }

    result.matrix = true;
    return result;
}

function determinant(m) {
    // TODO: Implement this
}

function inverse(m) {
    // TODO: Implement this
}

// Vector-Specific Operations
function length(v) {
    return Math.sqrt(dot(v, v));
}

function angle(u, v) {
    return Math.acos(dot(u, v) / (length(u) * length(v)));
}

function normalize(v) {
    return scale(v, 1.0 / length(v));
}

function dot(u, v) {
    var sum = 0.0;

    if (!Array.isArray(u) || !Array.isArray(v) || u.length != v.length) {
        throw "dot(): vectors are not the same dimension";
    }

    for (var i = 0; i < u.length; ++i) {
        sum += u[i] * v[i];
    }

    return sum;
}

function cross(u, v) {
    if (u.length != 3 || v.length != 3) {
        throw "cross(): vectors are not three-dimensional";
    }

    var result = [
        u[1] * v[2] - u[2] * v[1],
        u[2] * v[0] - u[0] * v[2],
        u[0] * v[1] - u[1] * v[0]
    ];

    return result;
}

function mix(u, v, s) {
    if (typeof s !== "number") {
        throw "mix: the last paramter " + s + " must be a number";
    }

    if (u.length != v.length) {
        throw "vector dimension mismatch";
    }

    var result = [];
    for (var i = 0; i < u.length; ++i) {
        result.push(v[i] * s + u[i] * (1.0 - s));
    }

    return result;
}

function rotate(point, axis, angle) {
    var normalizedAxis = normalize(axis);

    var x = point[0] * Math.pow(Math.sin(angle / 2), 2) * (Math.pow(normalizedAxis[0], 2) - Math.pow(normalizedAxis[1], 2) - Math.pow(normalizedAxis[2], 2))
        + point[0] * Math.pow(Math.cos(angle / 2), 2)
        - normalizedAxis[2] * point[1] * Math.sin(angle)
        + 2 * normalizedAxis[0] * normalizedAxis[1] * point[1] * Math.pow(Math.sin(angle / 2), 2)
        + normalizedAxis[1] * point[2] * Math.sin(angle)
        + 2 * normalizedAxis[0] * normalizedAxis[2] * point[2] * Math.pow(Math.sin(angle / 2), 2);

    // compute the y coordinate of the rotated point
    var y = normalizedAxis[2] * point[0] * Math.sin(angle)
        + 2 * normalizedAxis[0] * normalizedAxis[1] * point[0] * Math.pow(Math.sin(angle / 2), 2)
        + point[1] * Math.pow(Math.sin(angle / 2), 2) * (Math.pow(normalizedAxis[1], 2) - Math.pow(normalizedAxis[0], 2) - Math.pow(normalizedAxis[2], 2))
        + point[1] * Math.pow(Math.cos(angle / 2), 2)
        - normalizedAxis[0] * point[2] * Math.sin(angle)
        + 2 * normalizedAxis[1] * normalizedAxis[2] * point[2] * Math.pow(Math.sin(angle / 2), 2);

    // compute the z coordinate of the rotated point
    var z = -normalizedAxis[1] * point[0] * Math.sin(angle)
        + 2 * normalizedAxis[0] * normalizedAxis[2] * point[0] * Math.pow(Math.sin(angle / 2), 2)
        + normalizedAxis[0] * point[1] * Math.sin(angle)
        + 2 * normalizedAxis[1] * normalizedAxis[2] * point[1] * Math.pow(Math.sin(angle / 2), 2)
        + point[2] * Math.pow(Math.sin(angle / 2), 2) * (Math.pow(normalizedAxis[2], 2) - Math.pow(normalizedAxis[0], 2) - Math.pow(normalizedAxis[1], 2))
        + point[2] * Math.pow(Math.cos(angle / 2), 2);

    return vec3(x, y, z);
}

function rotate(point, axis, origin, angle) {
    return add(rotate(sub(point, origin), axis, angle), origin);
}

// Affine Transformations
function translationMatrix(x, y, z) {
    var matrix = mat4();

    matrix[0][3] = x;
    matrix[1][3] = y;
    matrix[2][3] = z;

    return matrix;
}

function rotationMatrix(axis, angle) {
    var matrix = mat4();

    var c = Math.cos(angle);
    var omc = 1.0 - c;
    var s = Math.sin(angle);

    var normalizedAxis = normalize(axis);
    var x = normalizedAxis[0];
    var y = normalizedAxis[1];
    var z = normalizedAxis[2];

    matrix[0][0] = x * x * omc + c;
    matrix[0][1] = x * y * omc - z * s;
    matrix[0][2] = x * z * omc + y * s;

    matrix[1][0] = x * y * omc + z * s;
    matrix[1][1] = y * y * omc + c;
    matrix[1][2] = y * z * omc - x * s;

    matrix[2][0] = x * z * omc - y * s;
    matrix[2][1] = y * z * omc + x * s;
    matrix[2][2] = z * z * omc + c;

    return matrix;
}

function scalingMatrix(x, y, z) {
    var matrix = mat4();

    matrix[0][0] = x;
    matrix[1][1] = y;
    matrix[2][2] = z;

    return matrix;
}

function reflectionMatrix(plane) {
    var matrix = mat4();

    var planeMagnitude = length([plane[0], plane[1], plane[2]]);
    var a = plane[0] / planeMagnitude;
    var b = plane[1] / planeMagnitude;
    var c = plane[2] / planeMagnitude;
    var d = plane[3] / planeMagnitude;

    matrix[0][0] = 1.0 - 2.0 * a * a;
    matrix[0][1] = -2.0 * a * b;
    matrix[0][2] = -2.0 * a * c;
    matrix[0][3] = -2.0 * a * d;

    matrix[1][0] = -2.0 * a * b;
    matrix[1][1] = 1.0 - 2.0 * b * b;
    matrix[1][2] = -2.0 * b * c;
    matrix[1][3] = -2.0 * b * d;

    matrix[2][0] = -2.0 * a * c;
    matrix[2][1] = -2.0 * b * c;
    matrix[2][2] = 1.0 - 2.0 * c * c;
    matrix[2][3] = -2.0 * c * d;

    return matrix;
}

function shearMatrix(hxy, hxz, hyx, hyz, hzx, hzy) {
    var matrix = mat4();

    matrix[0][1] = hxy;
    matrix[0][2] = hxz;

    matrix[1][0] = hyx;
    matrix[1][2] = hyz;

    matrix[2][0] = hzx;
    matrix[2][1] = hzy;

    return matrix;
}

// View and Projection Matrices
function lookAt(eye, spot, worldUp) {
    if (!Array.isArray(eye) || eye.length != 3) {
        throw "lookAt(): first parameter [eye] must be an a vec3";
    }

    if (!Array.isArray(spot) || spot.length != 3) {
        throw "lookAt(): second parameter [spot] must be an a vec3";
    }

    if (!Array.isArray(worldUp) || worldUp.length != 3) {
        throw "lookAt(): third parameter [wordUp] must be an a vec3";
    }

    if (equal(eye, spot)) {
        return mat4();
    }

    var look = normalize(sub(spot, eye));
    var right = normalize(cross(look, worldUp));
    var up = normalize(cross(right, look));

    look = negate(look);

    var result = mat4(
        vec4(right, -dot(right, eye)),
        vec4(up, -dot(up, eye)),
        vec4(look, -dot(look, eye)),
        vec4()
    );

    return result;
}

function ortho(left, right, bottom, top, near, far) {
    if (left == right) {
        throw "ortho(): left and right are equal";
    }
    if (bottom == top) {
        throw "ortho(): bottom and top are equal";
    }
    if (near == far) {
        throw "ortho(): near and far are equal";
    }

    var width = right - left;
    var height = top - bottom;
    var depth = far - near;

    var result = mat4();
    result[0][0] = 2.0 / width;
    result[1][1] = 2.0 / height;
    result[2][2] = -2.0 / depth;
    result[0][3] = -(left + right) / width;
    result[1][3] = -(top + bottom) / height;
    result[2][3] = -(far + near) / depth;

    return result;
}

function perspective(fov, aspectRatio, near, far) {
    var top = near * Math.tan(radians(fov) / 2.0);
    var right = top * aspectRatio;

    return frustum(-right, right, -top, top, near, far);
}

function frustum(left, right, bottom, top, near, far) {
    if (left == right) {
        throw "frustum(): left and right are equal";
    }
    if (bottom == top) {
        throw "frustum(): bottom and top are equal";
    }
    if (near == far) {
        throw "frustum(): near and far are equal";
    }
    if (near <= 0.0) {
        throw "frustum(): near is less than or equal to 0.0";
    }
    if (far <= 0.0) {
        throw "frustum(): far is less than or equal to 0.0";
    }

    var width = right - left;
    var height = top - bottom;
    var depth = far - near;

    var result = mat4();
    result[0][0] = 2.0 * near / width;
    result[1][1] = 2.0 * near / height;
    result[0][2] = (left + right) / width;
    result[1][2] = (top + bottom) / height;
    result[2][2] = -(far + near) / depth;
    result[3][2] = -1.0;
    result[2][3] = -2.0 * far * near / depth;

    return result;
}