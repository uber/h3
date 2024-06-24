import { latLngToCell } from "h3-js";
import React, { useCallback, useState } from "react";

export const WhereAmIButton = ({ setUserInput }) => {
  const [geolocationStatus, setGeolocationStatus] = useState("");

  const doGeoLocation = useCallback(async () => {
    if ("geolocation" in navigator) {
      setGeolocationStatus("Locating...");
      navigator.geolocation.getCurrentPosition(
        (position) => {
          // TODO: Consider using the accuracy to select resolution
          setUserInput(
            latLngToCell(
              position.coords.latitude,
              position.coords.longitude,
              11,
            ),
          );
          setGeolocationStatus("");
        },
        () => {
          setGeolocationStatus("Error");
        },
      );
    } else {
      setGeolocationStatus("No location services");
    }
  }, [setUserInput]);

  return (
    <div
      style={{
        position: "absolute",
        right: "12px",
        top: "12px",
      }}
    >
      <button
        type="button"
        onClick={doGeoLocation}
        title="Where am I?"
        disabled={Boolean(geolocationStatus)}
      >
        {geolocationStatus || (
          <img
            style={{ filter: "grayscale(1)" }}
            src="images/icon-high-precision.svg"
            alt="Where am I?"
            title="Where am I?"
          ></img>
        )}
      </button>
    </div>
  );
};
