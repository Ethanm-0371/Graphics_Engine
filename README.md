# Graphics Engine
A graphics engine for the Advanced Graphics Programming subject.

## Members
- Jonathan Cacay Llanes - [xGauss05](https://github.com/xGauss05)
- Ethan Martín Parra - [Ethanm-0371](https://github.com/Ethanm-0371)

## Repository
[https://github.com/Ethanm-0371/Graphics_Engine](https://github.com/Ethanm-0371/Graphics_Engine)

## Effect Techniques Applied
### Environment Mapping
#### Skybox
![image](https://github.com/user-attachments/assets/208fe6bb-e4e3-4120-ba61-f6f9ba2774a9)
![image](https://github.com/user-attachments/assets/57042110-d6fc-45fb-8ad2-eb64c3b42dcc)
![image](https://github.com/user-attachments/assets/6ab1417f-8d15-413b-a1ec-442b2e804d96)

#### Skybox reflection
![image](https://github.com/user-attachments/assets/94059b04-31eb-4e74-9794-e375bb8f4a78)

### Multipass Bloom
#### Bright colors
![image](https://github.com/user-attachments/assets/021348ed-3d53-4680-8e19-ba5aecd72af1)
#### Blur bright colors
![image](https://github.com/user-attachments/assets/3dba5477-ed58-42d4-82de-eb009c0641e8)
#### Scene + Blur colors with added Bloom
![image](https://github.com/user-attachments/assets/ac5068a2-5047-47b4-9c6b-b821e019f324)

## Controls
### Camera navigation
| Key | Description |
| :----: | :-----------: | 
| <code>RMB + WASD</code> | Camera free movement | 
| <code>MMB</code> | Camera vertical/horizontal panning | 
| <code>LMB</code> | Camera orbital movement | 
| <code>Mouse Wheel Up</code> | Zoom in | 
| <code>Mouse Wheel Down</code> | Zoom out | 

## Technical difficulties during development
- We encountered difficulties when implementing the skybox — it appeared inverted, and we had to make several adjustments to fix the issue.
- Regarding bloom, we spent a significant amount of time trying to figure out why the bloom image wasn't appearing in the framebuffer. It turned out that we were unbinding it before we could display the image.

## Things to be improved
We still need to implement skybox-based lighting, and it would be great to have transform components for entities and being modified in the Inspector, allowing us to add and remove them at runtime.

## Known errors
- Bloom is not applied to the entire image and so the skybox and others are not affected. This is most likely due to the bloom pass being executed before drawing the skybox, so it overrides it.
- A last minute error was introduced when implementing the bloom effect. Defferred rendering is the only rendering method that works properly right now. This is most likely due to changing color attachments to support the different render passes for bloom. A buffer or color attachment missmatch or missassignment must have been introduced.
