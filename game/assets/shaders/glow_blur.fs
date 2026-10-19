#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;

out vec4 finalColor;

void main() {
    vec2 texel = 1.0 / vec2(textureSize(texture0, 0));
    vec4 sum = vec4(0.0);
    sum += texture(texture0, fragTexCoord + texel * vec2(-1.0, -1.0)) * 0.0625;
    sum += texture(texture0, fragTexCoord + texel * vec2( 0.0, -1.0)) * 0.125;
    sum += texture(texture0, fragTexCoord + texel * vec2( 1.0, -1.0)) * 0.0625;
    sum += texture(texture0, fragTexCoord + texel * vec2(-1.0,  0.0)) * 0.125;
    sum += texture(texture0, fragTexCoord) * 0.25;
    sum += texture(texture0, fragTexCoord + texel * vec2( 1.0,  0.0)) * 0.125;
    sum += texture(texture0, fragTexCoord + texel * vec2(-1.0,  1.0)) * 0.0625;
    sum += texture(texture0, fragTexCoord + texel * vec2( 0.0,  1.0)) * 0.125;
    sum += texture(texture0, fragTexCoord + texel * vec2( 1.0,  1.0)) * 0.0625;
    finalColor = sum * fragColor;
}
