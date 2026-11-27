#import "MBXArcticProjection.h"

static const double MBXSemiMajorAxis = 6378137;
static const double MBXFlattening = 1 / 298.257223563;
static const double MBXCentralLongitude = -45;
static const double MBXTrueScaleLatitude = 70;

static double MBXEccentricitySquared(void) { return MBXFlattening * (2 - MBXFlattening); }

/// Snyder's t, the conformal latitude term, for a latitude in radians.
static double MBXConformalTerm(double latitude) {
  double e = sqrt(MBXEccentricitySquared());
  double sinLatitude = sin(latitude);
  return tan(M_PI_4 - latitude / 2) / pow((1 - e * sinLatitude) / (1 + e * sinLatitude), e / 2);
}

MLNCoordinateReferenceSystem *MBXArcticProjection(void) {
  double e2 = MBXEccentricitySquared();
  double trueScaleLatitude = MBXTrueScaleLatitude * M_PI / 180;
  double sinTrueScaleLatitude = sin(trueScaleLatitude);
  double m = cos(trueScaleLatitude) / sqrt(1 - e2 * sinTrueScaleLatitude * sinTrueScaleLatitude);
  double rhoPerT = MBXSemiMajorAxis * m / MBXConformalTerm(trueScaleLatitude);
  return [[MLNCoordinateReferenceSystem alloc] initWithName:@"EPSG:3413"
      tileMatrixSet:(MLNTileMatrixSet){{-4194304, 4194304}, 8388608}
      forward:^MLNCRSPosition(CLLocationCoordinate2D coordinate) {
        double rho = rhoPerT * MBXConformalTerm(coordinate.latitude * M_PI / 180);
        double angle = (coordinate.longitude - MBXCentralLongitude) * M_PI / 180;
        return (MLNCRSPosition){rho * sin(angle), -rho * cos(angle)};
      }
      inverse:^CLLocationCoordinate2D(MLNCRSPosition position) {
        double e4 = e2 * e2;
        double e6 = e4 * e2;
        double e8 = e6 * e2;
        double chi = M_PI_2 - 2 * atan(hypot(position.x, position.y) / rhoPerT);
        double latitude = chi + (e2 / 2 + 5 * e4 / 24 + e6 / 12 + 13 * e8 / 360) * sin(2 * chi) +
                          (7 * e4 / 48 + 29 * e6 / 240 + 811 * e8 / 11520) * sin(4 * chi) +
                          (7 * e6 / 120 + 81 * e8 / 1120) * sin(6 * chi) +
                          (4279 * e8 / 161280) * sin(8 * chi);
        return CLLocationCoordinate2DMake(
            latitude * 180 / M_PI, MBXCentralLongitude + atan2(position.x, -position.y) * 180 / M_PI);
      }];
}
