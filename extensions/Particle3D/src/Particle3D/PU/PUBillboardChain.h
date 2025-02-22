/****************************************************************************
 Copyright (C) 2013 Henry van Merode. All rights reserved.
 Copyright (c) 2015-2016 Chukong Technologies Inc.
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 https://axmol.dev/

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#pragma once

#include <vector>
#include "renderer/RenderState.h"
#include "renderer/MeshCommand.h"
#include "renderer/CallbackCommand.h"
#include "renderer/backend/Buffer.h"
#include "base/Object.h"
#include "math/Math.h"

namespace ax
{

class MeshCommand;
class GLProgramState;
class IndexBuffer;
class VertexBuffer;
class Texture2D;
class ParticleSystem3D;
class Renderer;

class PUBillboardChain
{

public:
    /** Contains the data of an element of the BillboardChain.
     */
    class Element
    {

    public:
        Element();

        Element(const Vec3& position, float width, float texCoord, const Vec4& colour, const Quaternion& orientation);

        Vec3 position;
        float width;
        /// U or V texture coord depending on options
        float texCoord;
        Vec4 color;

        // Only used when mFaceCamera == false
        Quaternion orientation;
    };
    typedef std::vector<Element> ElementList;

    /** Constructor
    @param name The name to give this object
    @param maxElements The maximum number of elements per chain
    @param numberOfChains The number of separate chain segments contained in this object
    @param useTextureCoords If true, use texture coordinates from the chain elements
    @param useVertexColours If true, use vertex colours from the chain elements
    @param dynamic If true, buffers are created with the intention of being updated
    */
    PUBillboardChain(std::string_view name,
                     std::string_view texFile = "",
                     size_t maxElements       = 20,
                     size_t numberOfChains    = 1,
                     bool useTextureCoords    = true,
                     bool useColours          = true,
                     bool dynamic             = true);
    /// destructor
    virtual ~PUBillboardChain();

    /** Set the maximum number of chain elements per chain
     */
    virtual void setMaxChainElements(size_t maxElements);
    /** Get the maximum number of chain elements per chain
     */
    virtual size_t getMaxChainElements() const { return _maxElementsPerChain; }
    /** Set the number of chain segments (this class can render multiple chains
        at once using the same material).
    */
    virtual void setNumberOfChains(size_t numChains);
    /** Get the number of chain segments (this class can render multiple chains
    at once using the same material).
    */
    virtual size_t getNumberOfChains() const { return _chainCount; }

    /** Sets whether texture coordinate information should be included in the
        final buffers generated.
    @note You must use either texture coordinates or vertex colour since the
        vertices have no normals and without one of these there is no source of
        colour for the vertices.
    */
    virtual void setUseTextureCoords(bool use);
    /** Gets whether texture coordinate information should be included in the
        final buffers generated.
    */
    virtual bool getUseTextureCoords() const { return _useTexCoords; }

    /** The direction in which texture coordinates from elements of the
        chain are used.
    */
    enum TexCoordDirection
    {
        /// Tex coord in elements is treated as the 'u' texture coordinate
        TCD_U,
        /// Tex coord in elements is treated as the 'v' texture coordinate
        TCD_V
    };
    /** Sets the direction in which texture coords specified on each element
        are deemed to run along the length of the chain.
    @param dir The direction, default is TCD_U.
    */
    virtual void setTextureCoordDirection(TexCoordDirection dir);
    /** Gets the direction in which texture coords specified on each element
        are deemed to run.
    */
    virtual TexCoordDirection getTextureCoordDirection() { return _texCoordDir; }

    /** Set the range of the texture coordinates generated across the width of
        the chain elements.
    @param start Start coordinate, default 0.0
    @param end End coordinate, default 1.0
    */
    virtual void setOtherTextureCoordRange(float start, float end);
    /** Get the range of the texture coordinates generated across the width of
        the chain elements.
    */
    virtual const float* getOtherTextureCoordRange() const { return _otherTexCoordRange; }

    /** Sets whether vertex colour information should be included in the
        final buffers generated.
    @note You must use either texture coordinates or vertex colour since the
        vertices have no normals and without one of these there is no source of
        colour for the vertices.
    */
    virtual void setUseVertexColours(bool use);
    /** Gets whether vertex colour information should be included in the
        final buffers generated.
    */
    virtual bool getUseVertexColours() const { return _useVertexColour; }

    /** Sets whether or not the buffers created for this object are suitable
        for dynamic alteration.
    */
    virtual void setDynamic(bool dyn);

    /** Gets whether or not the buffers created for this object are suitable
        for dynamic alteration.
    */
    virtual bool getDynamic() const { return _dynamic; }

    /** Add an element to the 'head' of a chain.
    @remarks
        If this causes the number of elements to exceed the maximum elements
        per chain, the last element in the chain (the 'tail') will be removed
        to allow the additional element to be added.
    @param chainIndex The index of the chain
    @param billboardChainElement The details to add
    */
    virtual void addChainElement(size_t chainIndex, const Element& billboardChainElement);
    /** Remove an element from the 'tail' of a chain.
    @param chainIndex The index of the chain
    */
    virtual void removeChainElement(size_t chainIndex);
    /** Update the details of an existing chain element.
    @param chainIndex The index of the chain
    @param elementIndex The element index within the chain, measured from
        the 'head' of the chain
    @param billboardChainElement The details to set
    */
    virtual void updateChainElement(size_t chainIndex, size_t elementIndex, const Element& billboardChainElement);
    /** Get the detail of a chain element.
    @param chainIndex The index of the chain
    @param elementIndex The element index within the chain, measured from
        the 'head' of the chain
    */
    virtual const Element& getChainElement(size_t chainIndex, size_t elementIndex) const;

    /** Returns the number of chain elements. */
    virtual size_t getNumChainElements(size_t chainIndex) const;

    /** Remove all elements of a given chain (but leave the chain intact). */
    virtual void clearChain(size_t chainIndex);
    /** Remove all elements from all chains (but leave the chains themselves intact). */
    virtual void clearAllChains();

    /** Sets whether the billboard should always be facing the camera or a custom direction
        set by each point element.
    @remarks
        Billboards facing the camera are useful for smoke trails, light beams, etc by
        simulating a cylinder. However, because of this property, wide trails can cause
        several artefacts unless the head is properly covered.
        Therefore, non-camera-facing billboards are much more convenient for leaving big
        trails of movement from thin objects, for example a sword swing as seen in many
        fighting games.
    @param faceCamera True to be always facing the camera (Default value: True)
    @param normalVector Only used when faceCamera == false. Must be a non-zero vector.
    This vector is the "point of reference" for each point orientation. For example,
    if normalVector is Vector3::UNIT_Z, and the point's orientation is an identity
    matrix, the segment corresponding to that point will be facing towards UNIT_Z
    This vector is internally normalized.
    */
    void setFaceCamera(bool faceCamera, const Vec3& normalVector = Vec3::UNIT_X);

    void setDepthTest(bool isDepthTest);
    void setDepthWrite(bool isDepthWrite);
    void setBlendFunc(const BlendFunc& blendFunc);

    void render(Renderer* renderer, const Mat4& transform, ParticleSystem3D* particleSystem);

    // Overridden members follow
    // void _updateRenderQueue(RenderQueue*);
    // void getRenderOperation(RenderOperation&);
    // virtual bool preRender(SceneManager* sm, RenderSystem* rsys);
    // void getWorldTransforms(Matrix4*) const;
    /// @copydoc MovableObject::visitRenderables

protected:
    /// Setup the STL collections
    virtual void setupChainContainers();
    /// Setup vertex declaration
    virtual void setupVertexDeclaration();
    // Setup buffers
    virtual void setupBuffers();
    /// Update the contents of the vertex buffer
    virtual void updateVertexBuffer(const Mat4& camMat);
    /// Update the contents of the index buffer
    virtual void updateIndexBuffer();

    void init(std::string_view texFile);

private:
    void onBeforeDraw();

    void onAfterDraw();

protected:
    /// Maximum length of each chain
    size_t _maxElementsPerChain;
    /// Number of chains
    size_t _chainCount;
    /// Use texture coords?
    bool _useTexCoords;
    /// Use vertex colour?
    bool _useVertexColour;
    /// Dynamic use?
    bool _dynamic;
    /// Is the vertex declaration dirty?
    bool _vertexDeclDirty;
    /// Do the buffers need recreating?
    bool _buffersNeedRecreating;
    /// Do the bounds need redefining?
    mutable bool _boundsDirty;
    /// Is the index buffer dirty?
    bool _indexContentDirty;
    /// Is the vertex buffer dirty?
    bool _vertexContentDirty;
    /// Texture coord direction
    TexCoordDirection _texCoordDir;
    /// Other texture coord range
    float _otherTexCoordRange[2];
    /// When true, the billboards always face the camera
    bool _faceCamera;
    /// Used when mFaceCamera == false; determines the billboard's "normal". i.e.
    /// when the orientation is identity, the billboard is perpendicular to this
    /// vector
    Vec3 _normalBase;

    /// The list holding the chain elements
    ElementList _chainElementList;

    /** Simple struct defining a chain segment by referencing a subset of
        the preallocated buffer (which will be mMaxElementsPerChain * mChainCount
        long), by it's chain index, and a head and tail value which describe
        the current chain. The buffer subset wraps at mMaxElementsPerChain
        so that head and tail can move freely. head and tail are inclusive,
        when the chain is empty head and tail are filled with high-values.
    */
    struct ChainSegment
    {
        /// The start of this chains subset of the buffer
        size_t start;
        /// The 'head' of the chain, relative to start
        size_t head;
        /// The 'tail' of the chain, relative to start
        size_t tail;
    };
    typedef std::vector<ChainSegment> ChainSegmentList;
    ChainSegmentList _chainSegmentList;

    /// Chain segment has no elements
    static const size_t SEGMENT_EMPTY;

    struct VertexInfo
    {
        Vec3 position;
        Vec2 uv;
        Vec4 color;
    };
    MeshCommand _meshCommand;
    RenderState::StateBlock _stateBlock;
    Texture2D* _texture                  = nullptr;
    backend::ProgramState* _programState = nullptr;
    backend::Buffer* _indexBuffer        = nullptr;  // index buffer
    backend::Buffer* _vertexBuffer       = nullptr;  // vertex buffer

    std::vector<VertexInfo> _vertices;
    std::vector<uint16_t> _indices;

    std::string _texFile;

    backend::UniformLocation _locColor;
    backend::UniformLocation _locTexture;
    backend::UniformLocation _locPMatrix;

    // renderer state cache variables
    bool _rendererDepthTestEnabled                 = true;
    backend::CompareFunction _rendererDepthCmpFunc = backend::CompareFunction::LESS;
    backend::CullMode _rendererCullMode            = backend::CullMode::BACK;
    backend::Winding _rendererWinding              = backend::Winding::COUNTER_CLOCK_WISE;
    bool _rendererDepthWrite                       = false;
};

}
