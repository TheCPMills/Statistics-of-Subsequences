import glm.*;
import static org.lwjgl.opengl.GL20C.*;

public class SpotLight extends Light {
    public vec3 position;
    public vec3 direction;
    public vec3 attenuation;
    public float innerCone;
    public float outerCone;

    public SpotLight(vec3 position, vec3 direction, vec3 attenuation, float innerConeAngle, float outerConeAngle, float ambience, vec4 color) {
        super(3, ambience, color);
        this.position = position;
        this.direction = direction;
        this.attenuation = attenuation;
        this.innerCone = (float) Math.cos(innerConeAngle * Math.PI / 180.0f);
        this.outerCone = (float) Math.cos(outerConeAngle * Math.PI / 180.0f);
    }

    public void update(Shader shader, String indexString) {
        glUniform1i(glGetUniformLocation(shader.id, "lightType" + indexString), type);
        glUniform1f(glGetUniformLocation(shader.id, "lightAmbience" + indexString), ambience);
        glUniform4f(glGetUniformLocation(shader.id, "lightColor" + indexString), color.x, color.y, color.z, color.w);
        glUniform3f(glGetUniformLocation(shader.id, "lightPosition" + indexString), position.x, position.y, position.z);

        glUniform3f(glGetUniformLocation(shader.id, "spotLightDirection"), direction.x, direction.y, direction.z);
        glUniform3f(glGetUniformLocation(shader.id, "spotLightAttenuation" + indexString), attenuation.x, attenuation.y, attenuation.z);
        glUniform1f(glGetUniformLocation(shader.id, "spotLightInnerCone" + indexString), innerCone);
        glUniform1f(glGetUniformLocation(shader.id, "spotLightOuterCone" + indexString), outerCone);
    }
}