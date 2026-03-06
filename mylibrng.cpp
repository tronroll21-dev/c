#include <oleauto.h>

extern "C" {

__declspec(dllexport) void ProcessArray(SAFEARRAY* psaIn, SAFEARRAY* psaOut) {
    long lowerRow, upperRow, lowerCol, upperCol;
    SafeArrayGetLBound(psaIn, 1, &lowerRow);
    SafeArrayGetUBound(psaIn, 1, &upperRow);
    SafeArrayGetLBound(psaIn, 2, &lowerCol);
    SafeArrayGetUBound(psaIn, 2, &upperCol);

    for (long i = lowerRow; i <= upperRow; i++) {
        for (long j = lowerCol; j <= upperCol; j++) {
            double val;
            long idx[2] = {i, j};
            SafeArrayGetElement(psaIn, idx, &val);

            double result = val * 2.0;  // <-- your processing here

            SafeArrayPutElement(psaOut, idx, &result);
        }
    }
}

} // extern "C"