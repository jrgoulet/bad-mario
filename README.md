#**SLUG**
---------
Your mission as Marco is to kill Mario.


----------


###**To-Do**

 - Enemy behavior
 - Add Jumping / Crouching / Knifing / Shooting
 - Text
 - Title Screen
 - Background
 - Sprites to process:
	 - Mario
	 - Goomba
	 - Spy Guy
	 - Princess



----------


###**Adding a New Sprite**
Navigate to sprite initialization in **main.c** and ensure to:

1. Add to sprites array by calling **new_Sprite()**.
 > new_Sprite(name, size, x, y, h, v, tile_index, priority)
 
2. Set sprite collision properties.
> sprite_collision_init(left, right, upper, lower, border)

3. Set sprite animation frames by calling **sprite_animation_init()**. If the sprite does not have frames for a specific action, just insert standing frames.
> **Frames stored:**
 1. Walking Start
 2. Walking End
 3. Attack Start
 4. Attack End
 5. Death Start
 6. Death End
 7. Stand Start
 8. Stand End

Set sprite behavior (coming soon).

