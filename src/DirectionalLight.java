import glm.*;
import static org.lwjgl.opengl.GL20C.*;

public class DirectionalLight extends Light {
	public vec3 direction;

    public DirectionalLight(vec3 direction, float ambience, vec4 color) {
        super(2, ambience, color);
        this.direction = direction;
    }

    public void update(Shader shader, String indexString) {
        glUniform1i(glGetUniformLocation(shader.id, "lightType" + indexString), type);
        glUniform1f(glGetUniformLocation(shader.id, "lightAmbience" + indexString), ambience);
        glUniform4f(glGetUniformLocation(shader.id, "lightColor" + indexString), color.x, color.y, color.z, color.w);
        glUniform3f(glGetUniformLocation(shader.id, "lightPosition" + indexString), 0.0f, 0.0f, 0.0f);

        glUniform3f(glGetUniformLocation(shader.id, "directionalLightDirection" + indexString), direction.x, direction.y, direction.z);
    }
}