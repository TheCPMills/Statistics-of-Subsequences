import glm.*;
import static org.lwjgl.opengl.GL20C.*;

public class Material {
    public vec3 ambientColor;
    public vec3 specularColor;
    public vec3 emissiveColor;
    public Float shininess;
    public Float refractionIndex;
    public Float opacity;

    public Material(vec3 ambientColor, vec3 specularColor, vec3 emissiveColor, Float shininess, Float refractionIndex, Float opacity) {
        this.ambientColor = ambientColor;
        this.specularColor = specularColor;
        this.emissiveColor = emissiveColor;
        this.shininess = shininess;
        this.refractionIndex = refractionIndex;
        this.opacity = opacity;
    }

    public void setUniforms(Shader shader) throws Exception {
        if (ambientColor == null || specularColor == null || shininess == null) {
            throw new Exception("Material is not fully defined. Please specify at least the ambient color, specular color, and shininess of the material.");
        }

        glUniform3f(glGetUniformLocation(shader.id, "materialAmbient"), ambientColor.x, ambientColor.y, ambientColor.z);
        glUniform3f(glGetUniformLocation(shader.id, "materialSpecular"), specularColor.x, specularColor.y, specularColor.z);
        glUniform1f(glGetUniformLocation(shader.id, "materialShininess"), shininess);

        if (emissiveColor != null) {
            glUniform3f(glGetUniformLocation(shader.id, "materialEmissive"), emissiveColor.x, emissiveColor.y, emissiveColor.z);
        }
        if (refractionIndex != null) {
            glUniform1f(glGetUniformLocation(shader.id, "materialRefractionIndex"), refractionIndex);
        }
        if (opacity != null) {
            glUniform1f(glGetUniformLocation(shader.id, "materialOpacity"), opacity);
        }
    }
}