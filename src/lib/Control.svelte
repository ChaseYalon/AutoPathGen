<script lang="ts">
	import { onMount, tick } from "svelte";
	import fieldUrl from "../../assets/field_v2.png";

	let canvas: HTMLCanvasElement;
	let ctx: CanvasRenderingContext2D;

	let imageHeight: number = $state(0);
	let imageWidth: number = $state(0);
    let container: HTMLDivElement;
    let bgImage: HTMLImageElement | null = null;

	function loadImage(url: string): Promise<HTMLImageElement> {
		return new Promise((resolve, reject) => {
			const img = new Image();
			img.decoding = "async";
			img.onload = () => resolve(img);
			img.onerror = reject;
			img.src = url;
		});
	}

    async function ensureImageLoaded() {
        if (!bgImage) {
            bgImage = await loadImage(fieldUrl);
        }
    }

    $effect(() => {
        if (canvas && container) {
            const resizeObserver = new ResizeObserver(() => {
                imageWidth = container.clientWidth;
                imageHeight = container.clientHeight;
                draw();
            });
            resizeObserver.observe(container);
            return () => resizeObserver.disconnect();
        }
    });

    async function draw() {
        if (!canvas || !imageWidth || !imageHeight) return;
        ctx = canvas.getContext("2d")!;
        await ensureImageLoaded();
        
        ctx.clearRect(0, 0, imageWidth, imageHeight);
        if (bgImage) {
            ctx.drawImage(bgImage, 0, 0, imageWidth, imageHeight);
        }
    }

	onMount(async () => {
        await tick();
        // Initial draw trigger if size is already known or handled by observer
	});
</script>

<div bind:this={container} style="width: 100%; height: 100%; overflow: hidden;">
    <canvas bind:this={canvas} width={imageWidth} height={imageHeight}></canvas>
</div>
