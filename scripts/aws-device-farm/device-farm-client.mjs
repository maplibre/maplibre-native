import {
  DeviceFarmClient,
} from "@aws-sdk/client-device-farm";

export function getDeviceFarmClient() {
  return new DeviceFarmClient({ region: "us-west-2" });
}
