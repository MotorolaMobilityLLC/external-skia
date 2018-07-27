/*
 * Copyright 2018 Google LLC All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.skar.examples.helloskar.rendering;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.BitmapShader;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorFilter;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffColorFilter;
import android.graphics.RectF;
import android.graphics.Shader;
import android.opengl.Matrix;

import com.google.ar.core.Plane;
import com.google.ar.core.PointCloud;
import com.google.ar.core.Pose;
import com.google.ar.core.TrackingState;
import com.google.skar.CanvasMatrixUtil;
import com.google.skar.PaintUtil;
import com.google.skar.examples.helloskar.app.FingerPainting;

import java.io.IOException;
import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.Collection;

/**
 * Sample class that handles drawing different types of geometry using the matrices provided
 * by ARCore. The matrices are handled by SkARMatrix in order to be passed to the drawing
 * Canvas.
 */

public class DrawManager {
    public enum DrawingType {
        circle, rect, text, animation
    }

    // App defaults
    public DrawManager.DrawingType currentDrawabletype = DrawManager.DrawingType.circle;
    public boolean drawSmoothPainting = true;

    private float[] projectionMatrix = new float[16];
    private float[] viewMatrix = new float[16];
    private float viewportWidth;
    private float viewportHeight;
    private ColorFilter lightFilter;
    private BitmapShader planeShader;
    public ArrayList<float[]> modelMatrices = new ArrayList<>();
    public FingerPainting fingerPainting = new FingerPainting(false);

    public void updateViewport(float width, float height) {
        viewportWidth = width;
        viewportHeight = height;
    }

    public void updateProjectionMatrix(float[] projectionMatrix) {
        this.projectionMatrix = projectionMatrix;
    }

    public void updateViewMatrix(float[] viewMatrix) {
        this.viewMatrix = viewMatrix;
    }

    public void updateLightColorFilter(float[] colorCorr) {
        lightFilter = PaintUtil.createLightCorrectionColorFilter(colorCorr);
    }

    // Sample function for drawing a circle
    public void drawCircle(Canvas canvas) {
        if (modelMatrices.isEmpty()) {
            return;
        }
        Paint p = new Paint();
        p.setColorFilter(lightFilter);
        p.setARGB(180, 100, 0, 0);

        canvas.save();
        android.graphics.Matrix m = CanvasMatrixUtil.createPerspectiveMatrix(modelMatrices.get(0),
                viewMatrix, projectionMatrix, viewportWidth, viewportHeight);
        canvas.setMatrix(m);

        canvas.drawCircle(0, 0, 0.1f, p);
        canvas.restore();
    }

    // Sample function for drawing an animated round rect
    public void drawAnimatedRoundRect(Canvas canvas, float radius) {
        if (modelMatrices.isEmpty()) {
            return;
        }
        Paint p = new Paint();
        p.setColorFilter(lightFilter);
        p.setARGB(180, 100, 0, 100);

        canvas.save();
        canvas.setMatrix(CanvasMatrixUtil.createPerspectiveMatrix(modelMatrices.get(0),
                viewMatrix, projectionMatrix, viewportWidth, viewportHeight));
        canvas.drawRoundRect(0,0, 0.5f, 0.5f, radius, radius, p);
        canvas.restore();
    }

    // Sample function for drawing a rect
    public void drawRect(Canvas canvas) {
        if (modelMatrices.isEmpty()) {
            return;
        }
        Paint p = new Paint();
        p.setColorFilter(lightFilter);
        p.setARGB(180, 0, 0, 255);
        canvas.save();
        canvas.setMatrix(CanvasMatrixUtil.createPerspectiveMatrix(modelMatrices.get(0),
                viewMatrix, projectionMatrix, viewportWidth, viewportHeight));
        RectF rect = new RectF(0, 0, 0.2f, 0.2f);
        canvas.drawRect(rect, p);
        canvas.restore();
    }

    // Sample function for drawing text on a canvas
    public void drawText(Canvas canvas, String text) {
        if (modelMatrices.isEmpty()) {
            return;
        }
        Paint p = new Paint();
        float textSize = 100;
        p.setColorFilter(lightFilter);
        p.setARGB(255, 0, 255, 0);
        p.setTextSize(textSize);

        float[] scaleMatrix = getTextScaleMatrix(textSize);
        float[] rotateMatrix = CanvasMatrixUtil.createXYtoXZRotationMatrix();
        float[][] matrices = { scaleMatrix, rotateMatrix, modelMatrices.get(0), viewMatrix,
                                projectionMatrix,
                CanvasMatrixUtil.createViewportMatrix(viewportWidth, viewportHeight)};

        canvas.save();
        canvas.setMatrix(CanvasMatrixUtil.createMatrixFrom4x4(CanvasMatrixUtil.multiplyMatrices4x4(matrices)));
        canvas.drawText(text, 0, 0, p);
        canvas.restore();
    }

    public void drawFingerPainting(Canvas canvas) {
        // Build the path before rendering
        fingerPainting.buildPath();

        // If path empty, return
        if (fingerPainting.getPaths().isEmpty()) {
            return;
        }

        // Get finger painting model matrix
        float[] model = fingerPainting.getModelMatrix();
        float[] in = new float[16];
        Matrix.setIdentityM(in, 0);
        Matrix.translateM(in, 0, model[12], model[13], model[14]);

        float[] initRot = CanvasMatrixUtil.createXYtoXZRotationMatrix();

        float[] scale = new float[16];
        float s = 0.001f;
        Matrix.setIdentityM(scale, 0);
        Matrix.scaleM(scale, 0, s, s, s);

        // Matrix = mvpv
        float[][] matrices = {scale, initRot, in, viewMatrix, projectionMatrix, CanvasMatrixUtil.createViewportMatrix(viewportWidth, viewportHeight)};
        android.graphics.Matrix mvpv = CanvasMatrixUtil.createMatrixFrom4x4(CanvasMatrixUtil.multiplyMatrices4x4(matrices));

        // Paint set up
        Paint p = new Paint();
        p.setStyle(Paint.Style.STROKE);
        p.setStrokeWidth(30f);
        p.setAlpha(120);

        for (FingerPainting.BuiltPath bp : fingerPainting.getPaths()) {
            if (bp.path.isEmpty()) {
                continue;
            }
            p.setColor(bp.color);

            // Scaling issues appear to happen when drawing a Path and transforming the Canvas
            // directly with a matrix on Android versions less than P. Ideally we would
            // switch true to be (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P)

            if (true) {
                // Transform applied through canvas
                canvas.save();
                canvas.setMatrix(mvpv);
                canvas.drawPath(bp.path, p);
                canvas.restore();
            } else {
                // Transform path directly
                Path pathDst = new Path();
                bp.path.transform(mvpv, pathDst);

                // Draw dest path
                canvas.save();
                canvas.setMatrix(new android.graphics.Matrix());
                canvas.drawPath(pathDst, p);
                canvas.restore();
            }
        }

    }

    // Sample function for drawing the AR point cloud
    public void drawPointCloud(Canvas canvas, PointCloud cloud) {
        FloatBuffer points = cloud.getPoints();
        int numberOfPoints = points.remaining() / 4;

        float[][] matrices = {viewMatrix, projectionMatrix, CanvasMatrixUtil.createViewportMatrix(viewportWidth, viewportHeight)};
        float[] vpv = CanvasMatrixUtil.multiplyMatrices4x4(matrices);

        float[] pointsToDraw = new float[numberOfPoints * 2];
        for (int i = 0; i < numberOfPoints; i++) {
            float[] point = {points.get(i * 4), points.get(i * 4 + 1), points.get(i * 4 + 2), 1};
            float[] result = CanvasMatrixUtil.multiplyMatrixVector(vpv, point, true);
            pointsToDraw[i * 2] = result[0];
            pointsToDraw[i * 2 + 1] = result[1];
        }

        Paint p = new Paint();
        p.setARGB(220, 20, 232, 255);
        p.setStrokeCap(Paint.Cap.SQUARE);
        p.setStrokeWidth(6.0f);

        canvas.save();
        float[] id = new float[16];
        Matrix.setIdentityM(id, 0);
        android.graphics.Matrix identity = CanvasMatrixUtil.createMatrixFrom4x4(id);
        canvas.setMatrix(identity);
        canvas.drawPoints(pointsToDraw, p);
        canvas.restore();
    }


    // Sample function for drawing AR planes
    public void drawPlanes(Canvas canvas, Pose cameraPose, Collection<Plane> allPlanes) {
        if (allPlanes.size() <= 0) {
            return;
        }

        for (Plane plane : allPlanes) {
            Plane subsumePlane = plane.getSubsumedBy();
            if (plane.getTrackingState() != TrackingState.TRACKING || subsumePlane != null) {
                continue;
            }

            float distance = calculateDistanceToPlane(plane.getCenterPose(), cameraPose);
            if (distance < 0) { // Plane is back-facing.
                continue;
            }

            // Get plane model matrix
            float[] model = new float[16];
            plane.getCenterPose().toMatrix(model, 0);

            // Initial rotation
            float[] initRot = CanvasMatrixUtil.createXYtoXZRotationMatrix();

            // Matrix = mvpv
            float[][] matrices = {initRot, model, viewMatrix, projectionMatrix, CanvasMatrixUtil.createViewportMatrix(viewportWidth, viewportHeight)};
            android.graphics.Matrix mvpv = CanvasMatrixUtil.createMatrixFrom4x4(CanvasMatrixUtil.multiplyMatrices4x4(matrices));

            drawPlaneAsPath(canvas, mvpv, plane);
        }
    }

    // Helper function that draws an AR plane using a path
    private void drawPlaneAsPath(Canvas canvas, android.graphics.Matrix mvpv, Plane plane) {
        int vertsSize = plane.getPolygon().limit() / 2;
        FloatBuffer polygon = plane.getPolygon();
        polygon.rewind();

        // Build source path from polygon data
        Path pathSrc = new Path();
        pathSrc.moveTo(polygon.get(0), polygon.get(1));
        for (int i = 1; i < vertsSize; i++) {
            pathSrc.lineTo(polygon.get(i * 2), polygon.get(i * 2 + 1));
        }
        pathSrc.close();

        // Set up paint
        Paint p = new Paint();

        if (false) {
            //p.setShader(planeShader);
            p.setColorFilter(new PorterDuffColorFilter(Color.argb(0.4f, 1, 0, 0),
                             PorterDuff.Mode.SRC_ATOP));
        }

        p.setColor(Color.RED);
        p.setAlpha(100);
        p.setStrokeWidth(0.01f);
        p.setStyle(Paint.Style.STROKE);


        if (true) {
            // Shader local matrix
            android.graphics.Matrix lm = new android.graphics.Matrix();
            lm.setScale(0.00005f, 0.00005f);
            planeShader.setLocalMatrix(lm);
            // Draw dest path
            canvas.save();
            canvas.setMatrix(mvpv);
            canvas.drawPath(pathSrc, p);
            canvas.restore();
        } else {
            // Build destination path by transforming source path
            Path pathDst = new Path();
            pathSrc.transform(mvpv, pathDst);

            // Shader local matrix
            android.graphics.Matrix lm = new android.graphics.Matrix();
            lm.setScale(0.00005f, 0.00005f);
            lm.postConcat(mvpv);
            planeShader.setLocalMatrix(lm);

            // Draw dest path
            canvas.save();
            canvas.setMatrix(new android.graphics.Matrix());
            canvas.drawPath(pathDst, p);
            canvas.restore();
        }
    }

    public void initializePlaneShader(Context context, String gridDistanceTextureName) throws IOException {
        // Read the texture.
        Bitmap planeTexture =
                BitmapFactory.decodeStream(context.getAssets().open(gridDistanceTextureName));
        // Set up the shader
        planeShader = new BitmapShader(planeTexture, Shader.TileMode.REPEAT, Shader.TileMode.REPEAT);
        planeShader.setLocalMatrix(new android.graphics.Matrix());
    }

    private float[] getTextScaleMatrix(float size) {
        float scaleFactor = 1 / (size * 10);
        float[] initScale = new float[16];
        android.opengl.Matrix.setIdentityM(initScale, 0);
        android.opengl.Matrix.scaleM(initScale, 0, scaleFactor, scaleFactor, scaleFactor);
        return initScale;
    }

    public static float calculateDistanceToPlane(Pose planePose, Pose cameraPose) {
        float[] normal = new float[3];
        float cameraX = cameraPose.tx();
        float cameraY = cameraPose.ty();
        float cameraZ = cameraPose.tz();
        // Get transformed Y axis of plane's coordinate system.
        planePose.getTransformedAxis(1, 1.0f, normal, 0);
        // Compute dot product of plane's normal with vector from camera to plane center.
        return (cameraX - planePose.tx()) * normal[0]
                + (cameraY - planePose.ty()) * normal[1]
                + (cameraZ - planePose.tz()) * normal[2];
    }
}
