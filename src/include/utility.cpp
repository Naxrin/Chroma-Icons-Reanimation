#include "utility.hpp"

// thanks to shadowforce78

void HSVtoRGB(float &fR, float &fG, float &fB, float &fH, float &fS, float &fV) {
    float fC = fV * fS; // Chroma
    float fHPrime = fmod(fH / 60.0, 6);
    float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
    float fM = fV - fC;

    if (0 <= fHPrime && fHPrime < 1) {
        fR = fC; fG = fX; fB = 0;
    } else if (1 <= fHPrime && fHPrime < 2) {
        fR = fX; fG = fC; fB = 0;
    } else if (2 <= fHPrime && fHPrime < 3) {
        fR = 0;  fG = fC; fB = fX;
    } else if (3 <= fHPrime && fHPrime < 4) {
        fR = 0;  fG = fX; fB = fC;
    } else if (4 <= fHPrime && fHPrime < 5) {
        fR = fX; fG = 0;  fB = fC;
    } else if (5 <= fHPrime && fHPrime < 6) {
        fR = fC; fG = 0;  fB = fX;
    } else {
        fR = 0;  fG = 0;  fB = 0;
    }

    fR += fM;
    fG += fM;
    fB += fM;
}
// RGB -> HSV
void RGBtoHSV(float &fR, float &fG, float &fB, float &fH, float &fS, float &fV){
    float delta;

    if (fR >= fG && fR >= fB){
        fV = fR;
        delta = fG >= fB ? fR-fB : fR-fG;
        fH = 60*fmod((fG-fB)/(delta), 6);
    }
    else if (fG >= fB && fG >= fR){
        fV = fG;
        delta = fB >= fR ? fG-fR : fG-fB;
        fH = 60*((fB-fR)/delta + 2);
    }
    else if (fB >= fR && fB >= fG){
        fV = fB;
        delta = fR >= fG ? fB-fG : fB-fR;
        fH = 60*((fR-fG)/delta + 4);
    }

    fS = fV == 0 ? 0 : delta / fV;
}

// get RGB cycle color
ccColor3B getRainbow(float &phase, float offset, float saturation) {
    float R, G, B;

    float hue = fmod(phase + offset, 360);
    float sat = saturation / 100.0;
    float vc = 1;
    HSVtoRGB(R, G, B, hue, sat, vc);

    cocos2d::ccColor3B out;

    out.r = R * 255;
    out.g = G * 255;
    out.b = B * 255;
    return out;
}

void fade(CCNode* node, bool in, float time, float scaleX, float scaleY, int opacity) {
    // convert default Val
    if (opacity < 0)
        opacity = in ? 255 : 0;
    if (scaleX < 0)
        scaleX = in ? 1 : 0.5;
    if (scaleY < 0)
        scaleY = in ? 1 : 0.5;

    auto action = CCSpawn::create(
        CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, opacity)),
        CCEaseExponentialOut::create(CCScaleTo::create(ANIM_TIME_M, scaleX, scaleY)),
        nullptr
    );
    if (in)
        node->setVisible(true);

    node->runAction(CCSequence::create(action,
        CallFuncExt::create([node, in](void) { node->setVisible(in); }),
        nullptr));
}

void fade(CCMenuItem* node, bool in, float time, float scaleX, float scaleY, int opacity) {
    // convert default Val
    if (opacity < 0)
        opacity = in ? 255 : 0;
    if (scaleX < 0)
        scaleX = in;
    if (scaleY < 0)
        scaleY = in;

    auto action = CCSpawn::create(
        CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, opacity)),
        CCEaseExponentialOut::create(CCScaleTo::create(ANIM_TIME_M, scaleX, scaleY)),
        nullptr
    );
    if (in)
        node->setVisible(true);
    else
        node->setEnabled(false);

    node->runAction(CCSequence::create(action,
        CallFuncExt::create([node](void) { node->setEnabled(true); }),
        nullptr));
}