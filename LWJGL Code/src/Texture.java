import java.nio.*;

import org.lwjgl.*;
import org.lwjgl.stb.*;
import static org.lwjgl.opengl.GL11C.*;
import static org.lwjgl.opengl.GL13C.*;
import static org.lwjgl.opengl.GL20C.*;
import static org.lwjgl.opengl.GL30C.*;

public class Texture {
    public int id;
    public String type;
    public int unit;
    public String texturePath = System.getProperty("user.dir") + "/res/textures/";

    public Texture(String filePath, String textureType, int slot) {
        this.type = textureType;

        IntBuffer imageWidth = BufferUtils.createIntBuffer(1);
        IntBuffer imageHeight = BufferUtils.createIntBuffer(1);
        IntBuffer imageColorChannels = BufferUtils.createIntBuffer(1);
        STBImage.stbi_set_flip_vertically_on_load(true);

        // Reads the image from a file and stores it in bytes
        ByteBuffer bytes = STBImage.stbi_load(texturePath + filePath, imageWidth, imageHeight, imageColorChannels, 0);
        int numColorChannels = imageColorChannels.get();

        id = glGenTextures(); // generate texture ID

        glActiveTexture(GL_TEXTURE0 + slot);
        unit = slot;

        glActiveTexture(slot); // activate texture unit
        glBindTexture(GL_TEXTURE_2D, id); // bind texture to texture unit

        // set interpolation type (Configures the type of algorithm that is used to make the image smaller or bigger)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); // GL_NEAREST_MIPMAP_LINEAR
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST); // GL_LINEAR

        // set texture mapping (Configures the way the texture repeats)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // extra lines for if GL_CLAMP_TO_BORDER is used for texture mapping
        // float flatColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
        // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, flatColor);

        // assign image to texture
        if (type.equals("normal")) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth.get(), imageHeight.get(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
        } else if (numColorChannels == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, imageWidth.get(), imageHeight.get(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
        } else if (numColorChannels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, imageWidth.get(), imageHeight.get(), 0, GL_RGB, GL_UNSIGNED_BYTE, bytes);
        } else if (numColorChannels == 1) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, imageWidth.get(), imageHeight.get(), 0, GL_RED, GL_UNSIGNED_BYTE, bytes);
        } else {
            throw new RuntimeException("Automatic Texture Type Recognition Failed: Unknown number of color channels: " + imageColorChannels.get());
        }

        glGenerateMipmap(GL_TEXTURE_2D); // generate mipmaps

        STBImage.stbi_image_free(bytes);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    public void textureUnit(Shader shader, String uniform, int unit) {
        int textureUnit = glGetUniformLocation(shader.id, uniform);
        shader.activate();
        glUniform1i(textureUnit, unit);
    }

    public void bind() {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, id);
    }

    public void unbind() {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    public void delete() {
        glDeleteTextures(id);
    }
}