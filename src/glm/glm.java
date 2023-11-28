package glm;

public class glm {
    public static mat4 lookAt(vec3 eye, vec3 spot, vec3 worldUp) {
        mat4 lookAt = new mat4();

        vec3 look = spot.sub(eye).normalize();
        vec3 right = cross(look, worldUp).normalize();
        vec3 up = cross(right, look).normalize();

        lookAt.set(0, new vec4(right.x, up.x, look.x, 0f));
        lookAt.set(1, new vec4(right.y, up.y, look.y, 0f));
        lookAt.set(2, new vec4(right.z, up.z, look.z, 0f));
        lookAt.set(3, new vec4(-dot(right, eye), -dot(up, eye), -dot(look, eye), 1f));

        return lookAt;
    }
    
    public static mat4 ortho(float left, float right, float bottom, float top, float near, float far) {
        mat4 ortho = new mat4();

        ortho.set(0, new vec4(2f / (right - left), 0f, 0f, 0f));
        ortho.set(1, new vec4(0f, 2f / (top - bottom), 0f, 0f));
        ortho.set(2, new vec4(0f, 0f, -2f / (far - near), 0f));
        ortho.set(3, new vec4(-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1f));

        return ortho;
    }

    public static mat4 perspective(float fov, float aspectRatio, float near, float far) {
        mat4 perspective = new mat4();

        fov = radians(fov);
        float top = near * (float) Math.tan(fov / 2f);
        float bottom = -top;
        float right = top * aspectRatio;
        float left = -right;

        perspective.set(0, new vec4((2f * near) / (right - left), 0f, 0f, 0f));
        perspective.set(1, new vec4(0f, (2f * near) / (top - bottom), 0f, 0f));
        perspective.set(2, new vec4((right + left) / (right - left), (top + bottom) / (top - bottom), -(far + near) / (far - near), -1f));
        perspective.set(3, new vec4(0f, 0f, -(2f * far * near) / (far - near), 0f));

        return perspective;
    }

    public static float dot(vec2 v1, vec2 v2) {
        return v1.x * v2.x + v1.y * v2.y;
    }

    public static float dot(vec3 v1, vec3 v2) {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    public static float dot(vec4 v1, vec4 v2) {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
    }

    public static vec3 cross(vec3 v1, vec3 v2) {
        float x = v1.y * v2.z - v1.z * v2.y;
        float y = v1.z * v2.x - v1.x * v2.z;
        float z = v1.x * v2.y - v1.y * v2.x;
        return new vec3(x, y, z);
    }

    public static float angle(vec2 v1, vec2 v2) {
        float dot = dot(v1, v2);
        float length = v1.length() * v2.length();
        return (float) Math.acos(dot / length);
    }

    public static float angle(vec3 v1, vec3 v2) {
        float dot = dot(v1, v2);
        float length = v1.length() * v2.length();
        return (float) Math.acos(dot / length);
    }

    public static float angle(vec4 v1, vec4 v2) {
        float dot = dot(v1, v2);
        float length = v1.length() * v2.length();
        return (float) Math.acos(dot / length);
    }

    public static float mix(float a, float b, float mixAmount) {
        return b * mixAmount + a * (1f - mixAmount);
    }

    public static vec2 mix(vec2 a, vec2 b, float mixAmount) {
        float x = mix(a.x, b.x, mixAmount);
        float y = mix(a.y, b.y, mixAmount);
        return new vec2(x, y);
    }

    public static vec3 mix(vec3 a, vec3 b, float mixAmount) {
        float x = mix(a.x, b.x, mixAmount);
        float y = mix(a.y, b.y, mixAmount);
        float z = mix(a.z, b.z, mixAmount);
        return new vec3(x, y, z);
    }

    public static vec4 mix(vec4 a, vec4 b, float mixAmount) {
        float x = mix(a.x, b.x, mixAmount);
        float y = mix(a.y, b.y, mixAmount);
        float z = mix(a.z, b.z, mixAmount);
        float w = mix(a.w, b.w, mixAmount);
        return new vec4(x, y, z, w);
    }

    public static vec3 rotate(vec3 point, vec3 axis, float angle) {
        vec3 normalizedAxis = axis.normalize(); // normalize the axis

        double x = point.x * Math.pow(Math.sin(angle / 2), 2) * (Math.pow(normalizedAxis.x, 2) - Math.pow(normalizedAxis.y, 2) - Math.pow(normalizedAxis.z, 2))
                + point.x * Math.pow(Math.cos(angle / 2), 2)
                - normalizedAxis.z * point.y * Math.sin(angle)
                + 2 * normalizedAxis.x * normalizedAxis.y * point.y * Math.pow(Math.sin(angle / 2), 2)
                + normalizedAxis.y * point.z * Math.sin(angle)
                + 2 * normalizedAxis.x * normalizedAxis.z * point.z * Math.pow(Math.sin(angle / 2), 2);

        // compute the y coordinate of the rotated point
        double y = normalizedAxis.z * point.x * Math.sin(angle)
                + 2 * normalizedAxis.x * normalizedAxis.y * point.x * Math.pow(Math.sin(angle / 2), 2)
                + point.y * Math.pow(Math.sin(angle / 2), 2) * (Math.pow(normalizedAxis.y, 2) - Math.pow(normalizedAxis.x, 2) - Math.pow(normalizedAxis.z, 2))
                + point.y * Math.pow(Math.cos(angle / 2), 2)
                - normalizedAxis.x * point.z * Math.sin(angle)
                + 2 * normalizedAxis.y * normalizedAxis.z * point.z * Math.pow(Math.sin(angle / 2), 2);

        // compute the z coordinate of the rotated point
        double z = -normalizedAxis.y * point.x * Math.sin(angle)
                + 2 * normalizedAxis.x * normalizedAxis.z * point.x * Math.pow(Math.sin(angle / 2), 2)
                + normalizedAxis.x * point.y * Math.sin(angle)
                + 2 * normalizedAxis.y * normalizedAxis.z * point.y * Math.pow(Math.sin(angle / 2), 2)
                + point.z * Math.pow(Math.sin(angle / 2), 2) * (Math.pow(normalizedAxis.z, 2) - Math.pow(normalizedAxis.x, 2) - Math.pow(normalizedAxis.y, 2))
                + point.z * Math.pow(Math.cos(angle / 2), 2);
                
        return new vec3((float) x, (float) y, (float) z);
    }

    public static float radians(float degrees) {
        return (float) Math.toRadians(degrees);
    }

    public static mat4 translationMatrix(vec3 offset) {
        mat4 translation = mat4.identity();

        vec4 column3 = new vec4(offset.x, offset.y, offset.z, 1f);

        translation.set(3, column3);
        return translation;
    }

    public static mat4 scalingMatrix(vec3 scaleFactor) {
        mat4 scaling = new mat4();

        vec4 column0 = new vec4(scaleFactor.x, 0f, 0f, 0f);
        vec4 column1 = new vec4(0f, scaleFactor.y, 0f, 0f);
        vec4 column2 = new vec4(0f, 0f, scaleFactor.z, 0f);
        vec4 column3 = new vec4(0f, 0f, 0f, 1f);

        scaling.set(column0, column1, column2, column3);
        return scaling;
    }

    public static mat4 rotationMatrix(vec3 axis, float angle) {
        mat4 rotation = new mat4();

        float x = axis.x;
        float y = axis.y;
        float z = axis.z;

        float c = (float) Math.cos(angle);
        float s = (float) Math.sin(angle);
        float t = 1f - c;
        float tx = t * x;
        float ty = t * y;
        float tz = t * z;
        float txy = tx * y;
        float txz = tx * z;
        float tyz = ty * z;
        float sx = s * x;
        float sy = s * y;
        float sz = s * z;

        vec4 column0 = new vec4(tx * x + c, txy - sz, txz + sy, 0f);
        vec4 column1 = new vec4(txy + sz, ty * y + c, tyz - sx, 0f);
        vec4 column2 = new vec4(txz - sy, tyz + sx, tz * z + c, 0f);
        vec4 column3 = new vec4(0f, 0f, 0f, 1f);

        rotation.set(column0, column1, column2, column3);

        return rotation;
    }
}
