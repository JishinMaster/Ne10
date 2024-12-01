#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <NE10.h>
#include <malloc.h>

#define FFT_SIZE 1024  // Define FFT size (must be a power of 2)
#define FIXED_POINT_SHIFT 20  // Scale factor for fixed-point representation to handle a wider range

void print_complex(ne10_fft_cpx_int32_t* data, int size, int max_display) {
    for (int i = 0; i < max_display; i++) {
        printf("Index %d: Real=%d, Imag=%d\n", i, data[i].r, data[i].i);
    }
}

void print_real(int32_t* data, int size, int max_display) {
    for (int i = 0; i < max_display; i++) {
        printf("Index %d: Real=%d\n", i, data[i]);
    }
}

int main() {
    // Generate a larger input signal with a wider value range
    int32_t input_real[FFT_SIZE];
    int32_t input_imag[FFT_SIZE] = {0};  // Imaginary part initialized to 0

    // Fill the input array with a sine wave
    for (int i = 0; i < FFT_SIZE; i++) {
        input_real[i] = (int32_t)(10000.0 * sinf(2.0f * M_PI * (float)(i) / (float)FFT_SIZE)); // Retain decimals until conversion
    }

	
    // Allocate memory for fixed-point FFT input and output
    ne10_fft_cpx_int32_t* input_data = (ne10_fft_cpx_int32_t*)memalign(64, FFT_SIZE * sizeof(ne10_fft_cpx_int32_t));
    ne10_fft_cpx_int32_t* output_data = (ne10_fft_cpx_int32_t*)memalign(64, FFT_SIZE * sizeof(ne10_fft_cpx_int32_t));

	print_real(input_real, FFT_SIZE, 10);
    // Convert real and imaginary parts into Ne10 complex fixed-point format
    for (int i = 0; i < FFT_SIZE; i++) {
        input_data[i].r = input_real[i] << FIXED_POINT_SHIFT;  // Scale to fixed-point
        input_data[i].i = input_imag[i] << FIXED_POINT_SHIFT;
    }
    printf("FFT Input (showing first 10 values):\n");
    print_complex(input_data, FFT_SIZE, 10);
    // Initialize Ne10 fixed-point FFT configuration
    ne10_fft_cfg_int32_t cfg = ne10_fft_alloc_c2c_int32_neon(FFT_SIZE);
    if (!cfg) {
        fprintf(stderr, "Failed to allocate FFT configuration.\n");
        free(input_data);
        free(output_data);
        return -1;
    }

    // Perform the FFT
    ne10_fft_c2c_1d_int32_neon(output_data, input_data, cfg, 0, 1);  // 0 for forward FFT, 1 for scaling

    // Print FFT output
    printf("FFT Output (showing first 10 values):\n");
    print_complex(output_data, FFT_SIZE, 10);

    // Perform the inverse FFT
    ne10_fft_c2c_1d_int32_neon(input_data, output_data, cfg, 1, 0);  // 1 for inverse FFT, 1 for scaling

    printf("\nIFFT Output (before scaling back, showing first 10 values):\n");
    print_complex(input_data, FFT_SIZE, 10);
	
    // Scale back to original fixed-point range
    for (int i = 0; i < FFT_SIZE; i++) {
        input_data[i].r = input_data[i].r >> FIXED_POINT_SHIFT;  // Scale back to original
        input_data[i].i = input_data[i].i >> FIXED_POINT_SHIFT;
    }

    // Print IFFT output after scaling back
    printf("\nIFFT Output (normalized and scaled back, showing first 10 values):\n");
    print_complex(input_data, FFT_SIZE, 10);

    // Clean up
    free(input_data);
    free(output_data);
    ne10_fft_destroy_c2c_int32(cfg);

    return 0;
}
