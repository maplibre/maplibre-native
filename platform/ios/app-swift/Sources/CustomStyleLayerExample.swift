import Foundation
import MapLibre
import MetalKit
import SwiftUI
import UIKit

// #-example-code(CustomStyleLayerExample)
struct CustomStyleLayerExample: UIViewRepresentable {
    func makeCoordinator() -> CustomStyleLayerExample.Coordinator {
        Coordinator(self)
    }

    final class Coordinator: NSObject, MLNMapViewDelegate {
        var control: CustomStyleLayerExample

        init(_ control: CustomStyleLayerExample) {
            self.control = control
        }

        func mapViewDidFinishLoadingMap(_ mapView: MLNMapView) {
            let mapOverlay = CustomStyleLayer(identifier: "test-overlay")
            let style = mapView.style!
            style.layers.append(mapOverlay)
        }
    }

    func makeUIView(context: Context) -> MLNMapView {
        let mapView = MLNMapView()
        mapView.delegate = context.coordinator
        return mapView
    }

    func updateUIView(_: MLNMapView, context _: Context) {}
}

class CustomStyleLayer: MLNCustomStyleLayer {
    private var pipelineState: MTLRenderPipelineState?
    private var depthStencilStateWithoutStencil: MTLDepthStencilState?

    override func didMove(to mapView: MLNMapView) {
        #if MLN_RENDER_BACKEND_METAL
            let resource = mapView.backendResource()

            let shaderSource = """
            #include <metal_stdlib>
            using namespace metal;

            typedef struct
            {
               vector_float2 position;
               vector_float4 color;
            } Vertex;

            struct RasterizerData
            {
               float4 position [[position]];
               float4 color;
            };

            struct Uniforms
            {
               float4x4 matrix;
            };

            vertex RasterizerData
            vertexShader(uint vertexID [[vertex_id]],
                        constant Vertex *vertices [[buffer(0)]],
                        constant Uniforms &uniforms [[buffer(1)]])
            {
               RasterizerData out;

               const float4 position = uniforms.matrix * float4(float2(vertices[vertexID].position.xy), 1, 1);

               out.position = position;
               out.color = vertices[vertexID].color;

               return out;
            }

            fragment float4 fragmentShader(RasterizerData in [[stage_in]])
            {
               return in.color;
            }
            """

            let device = resource.device
            var library: MTLLibrary? = nil
            var shaderError: Error? = nil
            do {
                library = try device?.makeLibrary(source: shaderSource, options: nil)
            } catch {
                shaderError = error
            }
            assert(library != nil, "Error compiling shaders: \(String(describing: shaderError))")
            let vertexFunction = library?.makeFunction(name: "vertexShader")
            let fragmentFunction = library?.makeFunction(name: "fragmentShader")

            // Configure a pipeline descriptor that is used to create a pipeline state.
            let pipelineStateDescriptor = MTLRenderPipelineDescriptor()
            pipelineStateDescriptor.label = "Simple Pipeline"
            pipelineStateDescriptor.vertexFunction = vertexFunction
            pipelineStateDescriptor.fragmentFunction = fragmentFunction
            pipelineStateDescriptor.colorAttachments[0].pixelFormat = resource.mtkView.colorPixelFormat
            pipelineStateDescriptor.depthAttachmentPixelFormat = .depth32Float_stencil8
            pipelineStateDescriptor.stencilAttachmentPixelFormat = .depth32Float_stencil8

            do {
                pipelineState = try device?.makeRenderPipelineState(descriptor: pipelineStateDescriptor)
            } catch {
                assertionFailure("Failed to create pipeline state: \(error)")
            }

            // Notice that we don't configure the stencilTest property, leaving stencil testing disabled
            let depthStencilDescriptor = MTLDepthStencilDescriptor()
            depthStencilDescriptor.depthCompareFunction = .always // Or another value as needed
            depthStencilDescriptor.isDepthWriteEnabled = false

            depthStencilStateWithoutStencil = device!.makeDepthStencilState(descriptor: depthStencilDescriptor)
        #endif
    }

    override func willMove(from _: MLNMapView) {}

    override func draw(in _: MLNMapView, with context: MLNStyleLayerDrawingContext) {
        #if MLN_RENDER_BACKEND_METAL
            guard let renderEncoder else { return }

            // Project to 0..1.
            let p1 = project(CLLocationCoordinate2D(latitude: 25.0, longitude: 12.5))
            let p2 = project(CLLocationCoordinate2D(latitude: 0.0, longitude: 0.0))
            let p3 = project(CLLocationCoordinate2D(latitude: 0.0, longitude: 25.0))

            // Multiply by the world size so it becomes the tile coordinate system.
            let worldSize = 512.0 * pow(2.0, context.zoomLevel)

            let p1Tile = CGPoint(x: p1.x * worldSize, y: p1.y * worldSize)
            let p2Tile = CGPoint(x: p2.x * worldSize, y: p2.y * worldSize)
            let p3Tile = CGPoint(x: p3.x * worldSize, y: p3.y * worldSize)

            // Then build a triangle from tile coordinates
            struct Vertex { var position: vector_float2; var color: vector_float4 }
            let triangleVertices: [Vertex] = [
                Vertex(position: vector_float2(Float(p1Tile.x), Float(p1Tile.y)),
                       color: vector_float4(1, 0, 0, 1)),
                Vertex(position: vector_float2(Float(p2Tile.x), Float(p2Tile.y)),
                       color: vector_float4(0, 1, 0, 1)),
                Vertex(position: vector_float2(Float(p3Tile.x), Float(p3Tile.y)),
                       color: vector_float4(0, 0, 1, 1)),
            ]

            // Use the camera's full projection matrix *unchanged*.
            var matrix = convertMatrix(context.projectionMatrix)

            // Encode
            renderEncoder.setRenderPipelineState(pipelineState!)
            renderEncoder.setDepthStencilState(depthStencilStateWithoutStencil)
            renderEncoder.setVertexBytes(triangleVertices, length: MemoryLayout<Vertex>.size * triangleVertices.count, index: 0)
            renderEncoder.setVertexBytes(&matrix, length: MemoryLayout<float4x4>.size, index: 1)

            // Draw the triangle.
            renderEncoder.drawPrimitives(type: .triangle, vertexStart: 0, vertexCount: 3)
        #endif
    }

    func project(_ coordinate: CLLocationCoordinate2D) -> CGPoint {
        // We project the coordinates into the space 0 to 1 and then scale these when drawing based on the current zoom level
        let worldSize = 1.0
        let x = (180.0 + coordinate.longitude) / 360.0 * worldSize
        let yi = log(tan((45.0 + coordinate.latitude / 2.0) * Double.pi / 180.0))
        let y = (180.0 - yi * (180.0 / Double.pi)) / 360.0 * worldSize

        return CGPoint(x: x, y: y)
    }

    struct MLNMatrix4f {
        var m00, m01, m02, m03: Float
        var m10, m11, m12, m13: Float
        var m20, m21, m22, m23: Float
        var m30, m31, m32, m33: Float
    }

    func convertMatrix(_ mat: MLNMatrix4) -> MLNMatrix4f {
        MLNMatrix4f(
            m00: Float(mat.m00), m01: Float(mat.m01), m02: Float(mat.m02), m03: Float(mat.m03),
            m10: Float(mat.m10), m11: Float(mat.m11), m12: Float(mat.m12), m13: Float(mat.m13),
            m20: Float(mat.m20), m21: Float(mat.m21), m22: Float(mat.m22), m23: Float(mat.m23),
            m30: Float(mat.m30), m31: Float(mat.m31), m32: Float(mat.m32), m33: Float(mat.m33)
        )
    }
}

// #-end-example-code
