using BepInEx;
using System;
using BepInEx.Configuration;
using System.Runtime.InteropServices;
using UnityEngine;
using Photon.Pun;


namespace GTMumbleLink
{
    [BepInPlugin(PluginInfo.GUID, PluginInfo.Name, PluginInfo.Version)]
    public class Plugin : BaseUnityPlugin
    {
        [DllImport("LinkHelper.dll")] // This is the most god awful method I have ever had to endure writing. Because of the interop with the C++ LinkHelper, we can't use Vector3s as they don't translate to the unmanaged side
        public static extern void updateMumble(string identity, string context, float fAvatarFrontX, float fAvatarFrontY, float fAvatarFrontZ, float fAvatarTopX, float fAvatarTopY, float fAvatarTopZ, float fAvatarPositionX, float fAvatarPositionY, float fAvatarPositionZ, float fCameraPositionX, float fCameraPositionY, float fCameraPositionZ, float fCameraFrontX, float fCameraFrontY, float fCameraFrontZ, float fCameraTopX, float fCameraTopY, float fCameraTopZ);
        [DllImport("LinkHelper.dll")]
        public static extern void initMumble();
#if DEBUG       
        [DllImport("LinkHelper.dll")]
        public static extern void testInterop(string str, float num);
#endif
        void Start()
        {
            HarmonyPatches.ApplyHarmonyPatches();
#if DEBUG
            testInterop("wysi", 727);
#endif
            initMumble();
        }

        void Update()
        {
            var id = PhotonNetwork.InRoom ? PhotonNetwork.CurrentRoom.Name + PhotonNetwork.CloudRegion : "NOROOM"; // TODO: Check if this causes people who aren't in rooms to hear
            var camPos = Camera.main.transform.position;                                                           // eachother's positional data. would be goofy as fuck and confusing
            var camFwrd = Camera.main.transform.forward;
            var camUp = Camera.main.transform.up;
            // We have to be absolutely positive everything we are using is valid.
            // Any exception within the LinkHelper that isn't handled properly will
            // cause the Unity player to hard crash and we do not want that
            if (PhotonNetwork.LocalPlayer.UserId != null && id != null && camPos != null && camFwrd != null && camUp != null)
            {
                Debug.Log($"Identity: {PhotonNetwork.LocalPlayer.UserId} - Context: {id}");
                Debug.Log($"Camera forward vector: {camFwrd.x}, {camFwrd.y}, {camFwrd.z}");
                Debug.Log($"Camera up vector: {camUp.x}, {camUp.y}, {camUp.z}");
                Debug.Log($"Camera position: {camPos.x}, {camPos.y}, {camPos.z}");
                updateMumble(PhotonNetwork.LocalPlayer.UserId, id, camFwrd.x, camFwrd.y, camFwrd.z, camUp.x, camUp.y, camUp.z, camPos.x, camPos.y, camPos.z, camPos.x, camPos.y, camPos.z, camFwrd.x, camFwrd.y, camFwrd.z, camUp.x, camUp.y, camUp.z);
            }
        }
    }
}
