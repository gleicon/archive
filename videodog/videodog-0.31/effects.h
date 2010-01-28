/* effects.h - reference to the functions */

void gamma_lookup( float, unsigned char *);
void gamma_apply(struct vd_video *, char *);
void threshold(struct vd_video *, unsigned char);
void sobel(struct vd_video *);
void laplace(struct vd_video *);
void noise(struct vd_video *, double );
void negative(struct vd_video *); 
void mirror(struct vd_video *);
void lowpass(struct vd_video *);
void highpass(struct vd_video *);
void mark_areas(struct vd_video *);
void channel_threshold(struct vd_video *); 
void predator(struct vd_video *);
void posterize(struct vd_video *, unsigned char );
void median_filter(struct vd_video *);
