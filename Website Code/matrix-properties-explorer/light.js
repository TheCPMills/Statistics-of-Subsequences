class Light {
    constructor(type, ambience, color) {
        this.type = type;
        this.ambience = ambience;
        this.color = color;
    }

    update(shader, indexString) {
        gl.uniform1i(gl.getUniformLocation(shader, "lightType" + indexString), this.type);
        gl.uniform1f(gl.getUniformLocation(shader, "lightAmbience" + indexString), this.ambience);
        gl.uniform4fv(gl.getUniformLocation(shader, "lightColor" + indexString), flatten(this.color));
        gl.uniform3fv(gl.getUniformLocation(shader, "lightPosition" + indexString), flatten(this.position));
    }

    static updateAll(shader, lights) {
        gl.uniform1i(gl.getUniformLocation(shader, "lightCount"), lights.length);
        for (var i = 0; i < lights.length; i++) {
            var light = lights[i];
            // var indexString = "[" + lights.indexOf(light) + "]";
            var indexString = "";
            light.update(shader, indexString);
        }
    }
}

class PointLight extends Light {
    constructor(position, attenuation, ambience, color) {
        super(1, ambience, color);
        this.position = position;
        this.attenuation = attenuation;
    }

    update(shader, indexString) {
        super.update(shader, indexString);
        gl.uniform3fv(gl.getUniformLocation(shader, "pointLightAttenuation" + indexString), flatten(this.attenuation));
    }
}

class DirectionalLight extends Light {
    constructor(direction, ambience, color) {
        super(2, ambience, color);
        this.direction = direction;
    }

    update(shader, indexString) {
        super.update(shader, indexString);
        gl.uniform3fv(gl.getUniformLocation(shader, "directionalLightDirection" + indexString), flatten(this.direction));
    }
}

class SpotLight extends Light {
    constructor(position, direction, attenuation, innerConeAngle, outerConeAngle, ambience, color) {
        super(3, ambience, color);
        this.position = position;
        this.direction = direction;
        this.attenuation = attenuation;
        this.innerConeAngle = innerConeAngle;
        this.outerConeAngle = outerConeAngle;
    }

    update(shader, indexString) {
        super.update(shader, indexString);
        gl.uniform3fv(gl.getUniformLocation(shader, "spotLightDirection"), flatten(this.direction));
        gl.uniform3fv(gl.getUniformLocation(shader, "spotLightAttenuation" + indexString), flatten(this.attenuation));
        gl.uniform1f(gl.getUniformLocation(shader, "spotLightInnerCone" + indexString), this.innerCone);
        gl.uniform1f(gl.getUniformLocation(shader, "spotLightOuterCone" + indexString), this.outerCone);
    }
}