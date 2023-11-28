import glm.*;
import static org.lwjgl.opengl.GL20C.*;

public class PointLight extends Light {
    public vec3 position;
    public vec3 attenuation;

    public PointLight(vec3 position, vec3 attenuation, float ambience, vec4 color) {
        super(1, ambience, color);
        this.position = position;
        this.attenuation = attenuation;
    }

    public void update(Shader shader, String indexString) {
        glUniform1i(glGetUniformLocation(shader.id, "lightType" + indexString), type);
        glUniform1f(glGetUniformLocation(shader.id, "lightAmbience" + indexString), ambience);
        glUniform4f(glGetUniformLocation(shader.id, "lightColor" + indexString), color.x, color.y, color.z, color.w);
        glUniform3f(glGetUniformLocation(shader.id, "lightPosition" + indexString), position.x, position.y, position.z);
        
        glUniform3f(glGetUniformLocation(shader.id, "pointLightAttenuation" + indexString), attenuation.x, attenuation.y, attenuation.z);
    }
}