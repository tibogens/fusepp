SET(ALL_SAMPLES
	minimal
)

FOREACH (mysample ${ALL_SAMPLES})
    ADD_SUBDIRECTORY(${mysample})
ENDFOREACH (mysample)
