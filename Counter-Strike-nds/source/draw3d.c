#include "main.h"
#include "grenade.h"
#include "collisions.h"
#include "ui.h"

// Draw top screen
void Draw3DScene(void)
{
    // Set camera for drawing
    NE_CameraUse(Camara);

    // Reset polygons Alpha/Light/Effect
    NE_PolyFormat(31, 0, NE_LIGHT_0, NE_CULL_BACK, NE_MODULATION);

    // render 3D if map insn't at screen
    if (!isShowingMap)
    {
        // Field of view end coordinates
        int t1x = (xWithoutYForOcclusionSide1 * 500 + localPlayer->xPos) * 8192.0;
        int t1z = (zWithoutYForOcclusionSide1 * 500 + localPlayer->zPos) * 8192.0;
        int t2x = (xWithoutYForOcclusionSide2 * 500 + localPlayer->xPos) * 8192.0;
        int t2z = (zWithoutYForOcclusionSide2 * 500 + localPlayer->zPos) * 8192.0;

        // Draw map
        for (int i = 0; i < AllZones[AllPlayers[0].CurrentOcclusionZone].ZoneCount; i++)
        {
            bool inFov = false; // Is the map part in the field of view of the player?
            // Force to render the map part where the player is
            if (checkZoneForOcclusion(&AllOcclusionZone[AllZones[AllPlayers[0].CurrentOcclusionZone].AllVisibleZones[i]], AllPlayers[0].PlayerModel->x, AllPlayers[0].PlayerModel->z))
                inFov = true;
            else
            {
                // Check if the map part is in the field of view of the player
                for (int i2 = 0; i2 < 4; i2++)
                {
                    // if (PointInTriangle(AllOcclusionZone[AllZones[AllPlayers[0].CurrentOcclusionZone].AllVisibleZones[i]].angles[i2].x, AllOcclusionZone[AllZones[AllPlayers[0].CurrentOcclusionZone].AllVisibleZones[i]].angles[i2].y, AllPlayers[0].xPos, AllPlayers[0].zPos, xWithoutYForOcclusionSide1 * 500 + localPlayer->xPos, zWithoutYForOcclusionSide1 * 500 + localPlayer->zPos, xWithoutYForOcclusionSide2 * 500 + localPlayer->xPos, zWithoutYForOcclusionSide2 * 500 + localPlayer->zPos))
                    if (PointInTriangleInt(AllOcclusionZone[AllZones[AllPlayers[0].CurrentOcclusionZone].AllVisibleZones[i]].anglesInt[i2].x, AllOcclusionZone[AllZones[AllPlayers[0].CurrentOcclusionZone].AllVisibleZones[i]].anglesInt[i2].y, AllPlayers[0].PlayerModel->x, AllPlayers[0].PlayerModel->z, t1x, t1z, t2x, t2z))
                    {
                        inFov = true;
                        break;
                    }
                }
            }
            // Render map model if needed
            if (inFov)
                NE_ModelDraw(Model[AllZones[AllPlayers[0].CurrentOcclusionZone].AllVisibleZones[i]]);
        }

        // for (int i = 0; i < 7; i++)
        // NE_ModelDraw(Model[i]);

        // Draw grenades and their effects
        for (int i = 0; i < GrenadeCount; i++)
        {
            if (grenades[i] != NULL)
            {
                // Draw grenade
                if (grenades[i]->isVisible)
                {
                    // Grenade clipping
                    bool inFov = PointInTriangleInt(grenades[i]->Model->x, grenades[i]->Model->z, AllPlayers[0].PlayerModel->x, AllPlayers[0].PlayerModel->z, t1x, t1z, t2x, t2z);

                    if (inFov)
                        NE_ModelDraw(grenades[i]->Model);
                }
                // Set polygons alpha
                NE_PolyFormat(grenades[i]->effectAlpha, 0, NE_LIGHT_0, NE_CULL_BACK, NE_MODULATION);
                // Draw effect if timer effect is on
                if (grenades[i]->EffectTimer != 0)
                {
                    if (grenades[i]->GrenadeType == 1 && isInFullSmoke)
                        continue;
                    NE_ModelDraw(grenades[i]->EffectModel);
                }
            }
        }
        isInFullSmoke = false;

        // Reset polygons Alpha/Light/Effect
        NE_PolyFormat(31, 0, NE_LIGHT_0, NE_CULL_BACK, NE_MODULATION);

        // If bomb is planted, draw bomb
        if (BombPlanted)
            NE_ModelDraw(Model[7]);

        // Show wall bullet hit flash
        if (ShowWallHitFlash > 0)
        {
            ShowWallHitFlash--;
            NE_ModelDraw(Model[8]);
        }

        // NE_ModelDraw(Model[9]);

        // Draw bomb explosion
        if (IsExplode)
        {
            // Calculate explosion effect alpha
            int ExplosionAlpha = 31 - (int)(BombExplosionScale / 1.2);
            if (ExplosionAlpha < 0)
            {
                ExplosionAlpha = 0;
                IsExplode = false;
            }
            // Set polygons alpha
            NE_PolyFormat(ExplosionAlpha, 0, NE_LIGHT_0, NE_CULL_BACK, NE_MODULATION);

            // Draw explosion
            NE_ModelDraw(Model[10]);

            // Reset polygons Alpha/Light/Effect
            NE_PolyFormat(31, 0, NE_LIGHT_0, NE_CULL_BACK, NE_MODULATION);
        }

        // Draw players
        for (int i = 1; i < MaxPlayer; i++)
        {
            // To draw a player, he need to exists and to be alive
            if (AllPlayers[i].Id != -1 && !AllPlayers[i].IsDead && CurrentCameraPlayer != i)
            {
                // Check players zones, to hide players in hidden zone
                for (int i2 = 0; i2 < AllZones[AllPlayers[0].CurrentOcclusionZone].ZoneCount; i2++)
                {
                    for (int i3 = 0; i3 < AllZones[AllPlayers[i].CurrentOcclusionZone].ZoneCount; i3++)
                    {
                        // If local player and other player are in visible zones
                        if (AllZones[AllPlayers[0].CurrentOcclusionZone].AllVisibleZones[i2] == AllZones[AllPlayers[i].CurrentOcclusionZone].AllVisibleZones[i3])
                        {
                            // Chec player clipping
                            CalculatePlayerPosition(i); // TODO REMOVE THIS?
                                                        // bool inFov = PointInTriangle(AllPlayers[i].xPos, AllPlayers[i].zPos, AllPlayers[0].xPos, AllPlayers[0].zPos, t1x, t1z, t2x, t2z);
                            bool inFov = PointInTriangleInt(AllPlayers[i].PlayerModel->x, AllPlayers[i].PlayerModel->z, AllPlayers[0].PlayerModel->x, AllPlayers[0].PlayerModel->z, t1x, t1z, t2x, t2z);
                            // Draw player
                            if (inFov)
                                NE_ModelDraw(AllPlayers[i].PlayerModel);

                            // Stop for
                            i2 = AllZones[AllPlayers[0].CurrentOcclusionZone].ZoneCount;
                            break;
                        }
                    }
                }
            }
        }
    }
    /*for (int i = 1; i < MaxPlayer; i++)
    {
        if (AllPlayers[i].Id != -1 && !AllPlayers[i].IsDead)
        {
            NE_ModelDraw(AllPlayers[i].PlayerModel);
        }
    }*/

    // Draw UI
    drawTopScreenUI();
}