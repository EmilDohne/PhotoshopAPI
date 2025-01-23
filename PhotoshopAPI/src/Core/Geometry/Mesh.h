#pragma once

#include "Macros.h"

#include <vector>
#include <memory>
#include <cmath>

#include "BoundingBox.h"
#include "MeshOperations.h"

#include <Eigen/Dense>

// If we compile with C++<20 we replace the stdlib implementation with the compatibility
// library
#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif


PSAPI_NAMESPACE_BEGIN


// Hash function for std::pair, required for unordered_map
struct pair_hash 
{
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const 
    {
        auto hash1 = std::hash<T1>{}(p.first);
        auto hash2 = std::hash<T2>{}(p.second);
        return hash1 ^ (hash2 << 1);
    }
};


namespace Geometry
{
    template <typename T>
    struct QuadMesh;


    template <typename T, size_t _Size>
    struct Face
    {
        Point2D<T> centroid(const QuadMesh<T>& mesh) const
        {
            // Initialize the sum of coordinates
            T sum_x = 0;
            T sum_y = 0;
            size_t count = 0;
            // Loop through each vertex index in the mesh
            for (const auto& idx : m_VertexIndices)
            {
                // Retrieve the vertex from the mesh
                const Vertex<T>& vertex = mesh.vertex(idx);
                // Accumulate the x and y coordinates
                sum_x += vertex.point().x;
                sum_y += vertex.point().y;
                ++count; // Count the number of vertices
            }

            // Calculate the centroid coordinates
            if (count == 0)
            {
                return Point2D<T>(0, 0); // Return a default value if no vertices are found
            }

            return Point2D<T>(sum_x / static_cast<T>(count), sum_y / static_cast<T>(count));
        }

        void bbox(BoundingBox<T> _bbox)
        {
            m_BoundingBox = _bbox;
        }

        const BoundingBox<T>& bbox() const
        {
            return m_BoundingBox;
        }

        size_t vertex_idx(size_t in_face_idx) const
        {
            return m_VertexIndices[in_face_idx];
        }

        size_t num_vertices() const noexcept
        {
            return m_VertexIndices.size();
        }

        constexpr size_t vertex_idx_checked(size_t in_face_idx) const
        {
            static_assert(in_face_idx > _Size - 1, "Index out of bounds");
            return m_VertexIndices[in_face_idx];
        }

        void vertex_indices(std::array<size_t, _Size> vertex_indices)
        {
            m_VertexIndices = vertex_indices;
        }

        std::array<size_t, _Size> vertex_indices() const
        {
            return m_VertexIndices;
        }

    private:
        std::array<size_t, _Size> m_VertexIndices{};
        BoundingBox<T> m_BoundingBox{};
    };


    template <typename T, size_t MaxFaces>
    struct OctreeNode
    {
        // The bounding box for this node
        BoundingBox<T> m_Bbox; 

        // Fixed-size array of face indices, default initializes to numeric limits max of size_t
        std::array<size_t, MaxFaces> m_FaceIndices;
        size_t m_NumFaces = 0;

        // Child nodes. These will be null if m_Leaf = true
        std::array<std::unique_ptr<OctreeNode<T, MaxFaces>>, 8> m_Children;

        // Tracks whether this node is a leaf
        bool m_IsLeaf; 


        OctreeNode(const BoundingBox<T> b) : m_Bbox(b), m_IsLeaf(true) 
        {
            m_FaceIndices.fill(std::numeric_limits<size_t>::max());
        }

        void subdivide()
        {
            // Create eight children with smaller bounding boxes
            Point2D<T> mid = (m_Bbox.minimum + m_Bbox.maximum) * 0.5;

            for (size_t i = 0; i < 8; ++i)
            {
                BoundingBox<T> child_bbox;
                child_bbox.minimum = (i & 1) ? mid : m_Bbox.minimum; // x
                child_bbox.maximum = (i & 1) ? m_Bbox.maximum : mid; // x

                child_bbox.minimum.y = (i & 2) ? mid.y : m_Bbox.minimum.y; // y
                child_bbox.maximum.y = (i & 2) ? m_Bbox.maximum.y : mid.y; // y

                m_Children[i] = std::make_unique<OctreeNode<T, MaxFaces>>(child_bbox);
            }

            // Mark this node as non-leaf after subdivision
            m_IsLeaf = false;
        }

        /// Recursively add the face index into the octree splitting if we cannot add it.
        /// throws an error if max_depth was set too low to accomodate all the elements
        void insert(const QuadMesh<T>& mesh, size_t face_index, size_t depth, size_t max_depth)
        {
            const auto face_bbox = mesh.face(face_index).bbox();
            if (!m_Bbox.in_bbox(face_bbox))
            {
                return;
            }

            // If the node is a leaf and can store more faces, add the face
            if (m_IsLeaf)
            {
                if (m_NumFaces < MaxFaces)
                {
                    m_FaceIndices[m_NumFaces++] = face_index; // Store the face index
                }
                // We limit the depth here to avoid a stack overflow
                if (m_NumFaces >= MaxFaces && depth < max_depth)
                {
                    subdivide(); // Subdivide if too many faces
                    ++depth;
                    for (size_t i = 0; i < m_NumFaces; ++i)
                    {
                        for (auto& child : m_Children)
                        {
                            child->insert(mesh, m_FaceIndices[i], depth, max_depth); // Reinsert faces into children
                        }
                    }

                    // Clear the current count
                    m_FaceIndices = {};
                    m_NumFaces = 0; 
                }
                else if (m_NumFaces >= MaxFaces)
                {
                    PSAPI_LOG_ERROR("Octree",
                        "Unable to construct octree with the given max_depth of %zu as we cannot create each node with the max number of face. Please increase the max depth or adjust the geometry",
                        max_depth
                    );
                }
            }
            else
            {
                // Insert into children
                ++depth;
                for (auto& child : m_Children)
                {
                    child->insert(mesh, face_index, depth, max_depth);
                }
            }
        }


        // Function to find faces containing a position
        void query(Point2D<T> position, std::span<const size_t>& result_faces) const
        {
            if (!m_Bbox.in_bbox(position)) // Only proceed if the position is within this node's bounding box
            {
                return;
            }
            if (m_IsLeaf) // If this is a leaf node
            {
                result_faces = std::span<const size_t>(m_FaceIndices.begin(), m_NumFaces);
                return;
            }

            for (const auto& child : m_Children)
            {
                if (child)
                {
                    child->query(position, result_faces);
                    // Return immediately after finding the first child that contains the position
                    if (!result_faces.empty())
                    {
                        return; // Exit once we have faces from one child
                    }
                }
            }
        }
    };

    template <typename T, size_t MaxFaces>
    class Octree
    {
    public:

        Octree() = default;

        Octree(const BoundingBox<T>& bbox, size_t max_depth = 16)
            : m_Root(std::make_unique<OctreeNode<T, MaxFaces>>(bbox)), m_MaxDepth(max_depth) {}

        void insert(const QuadMesh<T>& mesh, size_t face_index)
        {
            size_t depth = 0;
            m_Root->insert(mesh, face_index, depth, m_MaxDepth);
        }


        std::span<const size_t> query(Point2D<T> position) const
        {
            std::span<const size_t> faces;
            m_Root->query(position, faces);
            return faces;
        }

    private:
        std::unique_ptr<OctreeNode<T, MaxFaces>> m_Root;
        size_t m_MaxDepth = 8;
    };


    /// Mesh class for 2D Geometry representation, implemented with an Octree structure
    /// accelerating lookups and traversals. This currently only supports quadrilateral meshes and is very specific
    /// to the needs of the PhotoshopAPI. It is therefore recommended to use a more generic mesh library if further
    /// operations are wanted.
    template <typename T>
    struct QuadMesh
    {

        QuadMesh() = default;

        /// Generate a QuadMesh from a flat vector of points. These points must be in scanline order 
        QuadMesh(const std::vector<Point2D<T>>& points, size_t x_divisions, size_t y_divisions)
        {
            std::vector<Vertex<T>> vertices;
            vertices.reserve(points.size());

            for (size_t y = 0; y < y_divisions; ++y)
            {
                double v = static_cast<double>(y) / (y_divisions - 1);
                for (size_t x = 0; x < x_divisions; ++x)
                {
                    double u = static_cast<double>(x) / (x_divisions - 1);
                    size_t idx = y * x_divisions + x;

                    vertices.emplace_back(points[idx], Point2D<double>(u, v));
                }
            }

            initialize_mesh(vertices, x_divisions, y_divisions);
        }

        /// Generate a QuadMesh from a flat vector of vertices. These points must be in scanline order
        QuadMesh(const std::vector<Vertex<T>>& vertices, size_t x_divisions, size_t y_divisions)
        {
            initialize_mesh(vertices, x_divisions, y_divisions);
        }

        void move(Point2D<T> offset)
        {
            Operations::move(m_Vertices, offset);
            m_BoundingBox = BoundingBox<T>::compute(m_Vertices);

            rebuild_face_bboxes();
            rebuild_octree();
        }

        std::vector<Point2D<T>> points() const
        {
            std::vector<Point2D<T>> pts;
            pts.reserve(m_Vertices.size());

            for (auto vertex : m_Vertices)
            {
                pts.push_back(vertex.point());
            }

            return pts;
        }

        const Vertex<T>& vertex(size_t index) const
        {
            if (index == std::numeric_limits<size_t>::max())
            {
                PSAPI_LOG_ERROR("Mesh", "Unable to retrieve vertex as its index is not valid");
            }
            if (index > m_Vertices.size() - 1)
            {
                PSAPI_LOG_ERROR("Mesh", "Unable to retrieve vertex as its index is not valid, max allowed index %zu. Given index: %zu", m_Vertices.size() - 1, index);
            }

            return m_Vertices[index];
        }

        const std::vector<Vertex<T>>& vertices() const
        {
            return m_Vertices;
        }

        /// Update the vertices of the mesh, the _vertices parameter must have the same amount of vertices as the mesh
        /// as this method is only supposed to represent transformations applied to the vertices not a complete rebuild of the structure.
        /// 
        /// If you wish to do that please re-initialize the mesh.
        void vertices(std::vector<Vertex<T>>& _vertices)
        {
            if (vertices.size() != m_Vertices.size())
            {
                PSAPI_LOG_ERROR("Mesh",
                    "Unable to replace vertices with differently sized vertex vector. This method is only intended to update existing vertices. If you wish to to rebuild the mesh re-initialize it please.");
            }

            m_Vertices = vertices;
            m_BoundingBox = BoundingBox<T>::compute(vertices);
            rebuild_face_bboxes();
            rebuild_octree();
        }

        const Face<T, 4>& face(size_t index) const
        {
            if (index == std::numeric_limits<size_t>::max())
            {
                PSAPI_LOG_ERROR("Mesh", "Unable to retrieve face as its index is not valid");
            }
            if (index > m_Faces.size() - 1)
            {
                PSAPI_LOG_ERROR("Mesh", "Unable to retrieve face as its index is not valid, max allowed index %zu. Given index: %zu", m_Faces.size() - 1, index);
            }

            return m_Faces[index];
        }

        const std::vector<Face<T, 4>>& faces() const
        {
            return m_Faces;
        }

        /// Look up the mesh uv coordinate at the given point returning {-1, -1} if the 
        /// point does not lie on the mesh.
        Point2D<double> uv_coordinate(Point2D<T> position) const
        {
            if (!m_BoundingBox.in_bbox(position))
            {
                return Point2D<double>(-1.0, -1.0);
            }

            // Step 1: Identify the face (quad) containing `position`
            auto face_indices = m_Octree.query(position);
            for (size_t face_index : face_indices)
            {
                Face<T, 4> face = m_Faces[face_index];
                
                // Check if the position is within this face, we first check based on bbox as that is as faster operation
                // for rejecting false positives
                const auto& face_bbox = face.bbox();
                if (lambdas::in_bbox(position, face_bbox.minimum, face_bbox.maximum))
                {
                    // Retrieve vertex indices for the face's four corners
                    size_t v0_idx = face.vertex_idx(0);
                    size_t v1_idx = face.vertex_idx(1);
                    size_t v2_idx = face.vertex_idx(2);
                    size_t v3_idx = face.vertex_idx(3);

                    // Get vertex positions and their associated UVs
                    Vertex<T> v0 = m_Vertices[v0_idx];
                    Vertex<T> v1 = m_Vertices[v1_idx];
                    Vertex<T> v2 = m_Vertices[v2_idx];
                    Vertex<T> v3 = m_Vertices[v3_idx];

                    // Only now check if the point lies within the actual quad itself.
                    if (point_in_quad(position, v0.point(), v1.point(), v3.point(), v2.point()))
                    {
                        // Bilinear interpolation for UVs
                        return bilinear_interpolation_uv(position, v0, v1, v3, v2);
                    }
                }
            }

            return Point2D<double>(-1.0, -1.0); // No valid UV coordinate found
        }

        BoundingBox<T> bbox() const noexcept 
        { 
            return m_BoundingBox; 
        }

    private:

        std::vector<Vertex<T>>  m_Vertices;
        std::vector<Face<T, 4>> m_Faces;
        BoundingBox<T>          m_BoundingBox;
        Octree<T, 128>          m_Octree;

        /// Initialize the mesh for a given number of vertices
        void initialize_mesh(
            const std::vector<Vertex<T>>& vertices,
            size_t x_divisions, 
            size_t y_divisions
        )
        {
            PSAPI_PROFILE_FUNCTION();
            m_Vertices = vertices;
            m_BoundingBox = BoundingBox<T>::compute(m_Vertices);

            m_Faces.reserve((x_divisions - 1) * (y_divisions - 1));

            // Generate the faces and populate the vertex indices.
            for (size_t y = 0; y < y_divisions - 1; ++y)
            {
                for (size_t x = 0; x < x_divisions - 1; ++x)
                {
                    size_t v0_idx = y * x_divisions + x;        // top-left vertex
                    size_t v1_idx = v0_idx + 1;                 // top-right vertex
                    size_t v2_idx = v0_idx + x_divisions;       // bottom-left vertex
                    size_t v3_idx = v2_idx + 1;                 // bottom-right vertex

                    std::vector<Vertex<T>> face_vertices = { m_Vertices[v0_idx], m_Vertices[v1_idx], m_Vertices[v2_idx], m_Vertices[v3_idx] };

                    auto& created_face = m_Faces.emplace_back();
                    created_face.vertex_indices({ v0_idx, v1_idx, v2_idx, v3_idx });
                    created_face.bbox(BoundingBox<T>::compute(face_vertices));
                }
            }

            rebuild_octree();
        }

        /// Rebuild the bboxes of all the faces. This needs to be called after a transformation!
        void rebuild_face_bboxes()
        {
            for (Face<T, 4>& _face : m_Faces)
            {
                auto face_indices = _face.vertex_indices();
                std::vector<Vertex<T>> points = { m_Vertices[face_indices[0]], m_Vertices[face_indices[1]], m_Vertices[face_indices[2]],m_Vertices[face_indices[3]] };

                _face.bbox(BoundingBox<T>::compute(points));
            }
        }

        // Rebuild the acceleration structure
        void rebuild_octree()
        {
            PSAPI_PROFILE_FUNCTION();
            // Rebuild the octree with updated positions
            m_Octree = Octree<T, 128>(m_BoundingBox);
            for (size_t i = 0; i < m_Faces.size(); ++i)
            {
                m_Octree.insert(*this, i);
            }
        }


        /// Compute whether a point is in a triangle defined by the three vertices vtx1, vtx2 and vtx3
        bool point_in_triangle(Point2D<T> point, Point2D<T> vtx1, Point2D<T> vtx2, Point2D<T> vtx3) const
        {
            auto sign = [](Point2D<T> p1, Point2D<T> p2, Point2D<T> p3) 
            {
                return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
            };

            // Compute the signed area values for each sub-triangle formed by the point and two triangle vertices.
            T d1 = sign(point, vtx1, vtx2);
            T d2 = sign(point, vtx2, vtx3);
            T d3 = sign(point, vtx3, vtx1);

            // Check if the point has both positive and negative signs relative to the triangle edges.
            // If both are present, the point is outside the triangle; otherwise, it's inside or on the edge.
            bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
            bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

            // The point is within the triangle if it does not have both positive and negative signs.
            return !(has_neg && has_pos);
        }


        bool point_in_quad(Point2D<T> p, Point2D<T> v0, Point2D<T> v1, Point2D<T> v3, Point2D<T> v2) const
        {
            // Check if the point is inside either of the triangles formed by splitting the quad
            return point_in_triangle(p, v0, v1, v3) || point_in_triangle(p, v0, v2, v3);
        }


        template<typename T>
        std::tuple<double, double, double> barycentric_coordinates(Point2D<T> p, Point2D<T> a, Point2D<T> b, Point2D<T> c) const
        {
            auto dot = [](Point2D<T> a, Point2D<T> b)
            {
                return a.x * b.x + a.y * b.y;
            };

            Point2D<double> v0 = b - a;
            Point2D<double> v1 = c - a;
            Point2D<double> v2 = p - a;

            double d00 = dot(v0, v0);
            double d01 = dot(v0, v1);
            double d11 = dot(v1, v1);
            double d20 = dot(v2, v0);
            double d21 = dot(v2, v1);
            double denom = d00 * d11 - d01 * d01;
            auto v = (d11 * d20 - d01 * d21) / denom;
            auto w = (d00 * d21 - d01 * d20) / denom;
            auto u = 1.0f - v - w;
            return std::make_tuple(u, v, w);
        }


        Point2D<double> bilinear_interpolation_uv(Point2D<T> p, Vertex<T> v0, Vertex<T> v1, Vertex<T> v3, Vertex<T> v2) const
        {
            // Check if point p is in the first triangle (v0, v1, v2)
            if (point_in_triangle(p, v0.point(), v1.point(), v3.point()))
            {
                auto [u, v, w] = barycentric_coordinates(p, v0.point(), v1.point(), v3.point());
                return (v0.uv() * u) + (v1.uv() * v) + (v3.uv() * w);
            }
            else
            {
                auto [u, v, w] = barycentric_coordinates(p, v0.point(), v2.point(), v3.point());
                return (v0.uv() * u) + (v2.uv() * v) + (v3.uv() * w);
            }
        }
    };
}

PSAPI_NAMESPACE_END