#define pi 3.141592654

int pow(int base, int deg) {
    int t = base;
    for (int i=0;i<deg;i++) {
        t = t * t;
    }
    return t;
}

float fpow(float base, int deg) {
    float t = base;
    for (int i=0;i<deg;i++) {
        t = t * t;
    }
    return t;
}

// rad * (180 / pi)
float deg(float rad) {
    return rad * 57.2957795131;
}

// deg * (pi / 180)
float rad(float deg) {
    return deg * 0.0174532925199;
}


float sin(float rad) {

}

float cos(float rad) {

}