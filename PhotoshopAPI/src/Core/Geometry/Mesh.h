#pragma once

#include "Macros.h"

#include <vector>
#include <memory>
#include <cmath>


#include "BoundingBox.h"


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
    struct Mesh;
    template <typename T>
    struct Vertex;
    struct Face;



    template <typename T>
    struct OctreeNode
    {
        BoundingBox<T> m_Bbox; // The bounding box for this node
        std::vector<size_t> m_FaceIndices; // Indices of faces contained in this node
        std::array<std::unique_ptr<OctreeNode>, 8> m_Children; // Child nodes
        size_t m_MaxFacesPerNode; // Maximum faces per node before splitting

        OctreeNode(const BoundingBox<T>& b, size_t max_faces)
            : m_Bbox(b), m_MaxFacesPerNode(max_faces) {}

        bool is_leaf() const
        {
            return m_Children[0] == nullptr;
        }

        void subdivide()
        {
            // Create eight children with smaller bounding boxes
            // Calculate the midpoints in x, y, z directions
            Point2D<T> mid = (m_Bbox.minimum + m_Bbox.maximum) * 0.5;

            for (size_t i = 0; i < 8; ++i)
            {
                BoundingBox<T> child_bbox;
                child_bbox.minimum = (i & 1) ? mid : m_Bbox.minimum; // x
                child_bbox.maximum = (i & 1) ? m_Bbox.maximum : mid; // x

                child_bbox.minimum.y = (i & 2) ? mid.y : m_Bbox.minimum.y; // y
                child_bbox.maximum.y = (i & 2) ? m_Bbox.maximum.y : mid.y; // y

                m_Children[i] = std::make_unique<OctreeNode>(child_bbox, m_MaxFacesPerNode);
            }
        }

        void insert(const Mesh<T>& mesh, size_t face_index)
        {
            const auto bbox = mesh.face(face_index).bbox(mesh);
            if (!m_Bbox.in_bbox(bbox))
            {
                return;
            };

            // If the node is a leaf and can store more faces, add the face
            if (is_leaf()) 
            {
                m_FaceIndices.push_back(face_index);
                if (m_FaceIndices.size() > m_MaxFacesPerNode) 
                {
                    subdivide(); // Subdivide if too many faces
                    for (size_t idx : m_FaceIndices) 
                    {
                        for (auto& child : m_Children) 
                        {
                            child->insert(mesh, idx); // Reinsert faces into children
                        }
                    }
                    m_FaceIndices.clear(); // Clear the current list
                }
            }
            else {
                // Insert into children
                for (auto& child : m_Children) 
                {
                    child->insert(mesh, face_index);
                }
            }
        }

        // Function to find faces containing a position
        void query(const Point2D<T>& position, std::vector<size_t>& result_faces) const
        {
            if (!m_Bbox.in_bbox(position)) 
            {
                return; // Position is outside this node's bounding box
            }

            // If leaf, add faces
            if (is_leaf()) {
                result_faces.insert(result_faces.end(), m_FaceIndices.begin(), m_FaceIndices.end());
            }
            else {
                // Query children
                for (const auto& child : m_Children) {
                    if (child) {
                        child->query(position, result_faces);
                    }
                }
            }
        }
    };

    template <typename T>
    class Octree
    {
    public:

        Octree() = default;

        Octree(const BoundingBox<T>& bbox, size_t max_faces_per_node)
            : m_Root(std::make_unique<OctreeNode<T>>(bbox, max_faces_per_node)) {}

        void insert(const Mesh<T>& mesh, size_t face_index)
        {
            m_Root->insert(mesh, face_index);
        }

        std::vector<size_t> query(const Point2D<T>& position) const
        {
            std::vector<size_t> result_faces;
            m_Root->query(position, result_faces);
            return result_faces;
        }

    private:
        std::unique_ptr<OctreeNode<T>> m_Root;
    };



    struct HalfEdge
    {
        HalfEdge() = default;

        /// Check if the given half edge is valid, this means all the indices are
        /// not uninitialized
        bool valid() const noexcept
        {
            return (
                m_Vertex != std::numeric_limits<size_t>::max() &&
                m_PointedAtIdx != std::numeric_limits<size_t>::max() &&
                m_OppositeIdx != std::numeric_limits<size_t>::max() &&
                m_NextIdx != std::numeric_limits<size_t>::max() &&
                m_FaceIdx != std::numeric_limits<size_t>::max()
            );
        }


        template <typename T>
        Vertex<T>& vertex(Mesh<T>& mesh)
        {
            return mesh.vertex(m_Vertex);
        }

        template <typename T>
        const Vertex<T>& vertex(const Mesh<T>& mesh) const
        {
            return mesh.vertex(m_Vertex);
        }

        size_t vertex_idx() const noexcept
        {
            return m_Vertex;
        }

        void vertex(size_t idx) noexcept
        {
            m_Vertex = idx;
        }

        template <typename T>
        Vertex<T>& pointed_at(Mesh<T>& mesh)
        {
            return mesh.vertex(m_PointedAtIdx);
        }

        template <typename T>
        const Vertex<T>& pointed_at(const Mesh<T>& mesh) const
        {
            return mesh.vertex(m_PointedAtIdx);
        }

        size_t pointed_at_idx() const noexcept
        {
            return m_PointedAtIdx;
        }

        void pointed_at(size_t idx) noexcept
        {
            m_PointedAtIdx = idx;
        }

        template <typename T>
        HalfEdge& next(Mesh<T>& mesh)
        {
            return mesh.half_edge(m_NextIdx);
        }

        size_t next_idx() const noexcept
        {
            return m_NextIdx;
        }

        void next(size_t idx) noexcept
        {
            m_NextIdx = idx;
        }

        template <typename T>
        HalfEdge& opposite(Mesh<T>& mesh)
        {
            return mesh.half_edge(m_OppositeIdx);
        }

        size_t opposite_idx() const noexcept
        {
            return m_OppositeIdx;
        }

        void opposite(size_t idx) noexcept
        {
            m_OppositeIdx = idx;
        }

        template <typename T>
        Face& face(Mesh<T>& mesh)
        {
            return mesh.face(m_FaceIdx);
        }

        size_t face_idx() const noexcept
        {
            return m_FaceIdx;
        }

        void face(size_t idx) noexcept
        {
            m_FaceIdx = idx;
        }
        
    private:
        size_t m_Vertex         = std::numeric_limits<size_t>::max();   // The vertex this half-edge starts at
        size_t m_PointedAtIdx   = std::numeric_limits<size_t>::max();   // The vertex this half-edge points to
        size_t m_OppositeIdx    = std::numeric_limits<size_t>::max();   // The opposite half-edge
        size_t m_NextIdx        = std::numeric_limits<size_t>::max();   // The next half-edge in the face
        size_t m_FaceIdx        = std::numeric_limits<size_t>::max();   // The face this half-edge belongs to
    };

    template <typename T>
    struct Vertex
    {
        Vertex(Point2D<T> point) : m_Point(point) {}
        Vertex(Point2D<T> point, Point2D<double> uv) : m_Point(point), m_UV(uv) {}

        Point2D<T> point() const
        {
            return m_Point;
        }

        Point2D<double> uv() const
        {
            return m_UV;
        }

    private:
        Point2D<T> m_Point;
        Point2D<double> m_UV = { -1.0, -1.0 };
        size_t m_HalfEdgeIdx = std::numeric_limits<size_t>::max();
    };


    struct Face 
    {

        template <typename T>
        Point2D<T> centroid(const Mesh<T>& mesh) const
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


        template <typename T>
        HalfEdge& half_edge(Mesh<T>& mesh)
        {
            mesh.half_edge(m_HalfEdgeIdx);
        }

        void half_edge(size_t idx) noexcept
        {
            m_HalfEdgeIdx = idx;
        }

        template <typename T>
        BoundingBox<T> bbox(const Mesh<T>& mesh) const
        {
            BoundingBox<T> bbox;
            // Initialize the bounding box to extreme values
            bbox.minimum = Point2D<T>(std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
            bbox.maximum = Point2D<T>(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest());

            for (const auto idx: m_VertexIndices)
            {
                auto point = mesh.vertex(idx).point();
                // Update min and max UV for bounding box
                bbox.minimum.x = std::min(bbox.minimum.x, point.x);
                bbox.minimum.y = std::min(bbox.minimum.y, point.y);
                bbox.maximum.x = std::max(bbox.maximum.x, point.x);
                bbox.maximum.y = std::max(bbox.maximum.y, point.y);
            }
            return bbox;
        }

        size_t vertex_idx(size_t in_face_idx) const noexcept
        {
            return m_VertexIndices[in_face_idx];
        }

        size_t num_vertices() const noexcept
        {
            return m_VertexIndices.size();
        }

        size_t vertex_idx_checked(size_t in_face_idx) const
        {
            return m_VertexIndices.at(in_face_idx);
        }

        void vertex_indices(std::vector<size_t> vertex_indices)
        {
            m_VertexIndices = std::move(vertex_indices);
        }

        std::vector<size_t> vertex_indices()
        {
            return m_VertexIndices;
        }

    private:
        std::vector<size_t> m_VertexIndices;
        size_t m_HalfEdgeIdx = std::numeric_limits<size_t>::max();
    };


    /// Mesh class for 2D Geometry representation, implemented as a modified half-edge data structure with an Octree
    /// accelerating lookups and traversals. This currently only supports quadrilateral meshes and is very specific
    /// to the needs of the PhotoshopAPI. It is therefore recommended to use a more generic mesh library if further
    /// operations are wanted. This structure is created once and cannot be modified again allowing for some optimizations
    /// 
    template <typename T>
    struct Mesh
    {
        /// Generate a HalfEdgeMesh from a flat vector of points. These points must be in scanline order and the 
        /// generated mesh is quadrilateral.
        Mesh(const std::vector<Point2D<T>> points, size_t x_divisions, size_t y_divisions)
        {
            BoundingBox<T> bbox;
            // Initialize the bounding box to extreme values
            bbox.minimum = Point2D<T>(std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
            bbox.maximum = Point2D<T>(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest());

            // Create vertices with UV coordinates, assumes the points are equally distributed
            m_Vertices.reserve(x_divisions * y_divisions);
            for (size_t y = 0; y < y_divisions; ++y)
            {
                double v = static_cast<double>(y) / (y_divisions - 1);
                for (size_t x = 0; x < x_divisions; ++x)
                {
                    double u = static_cast<double>(x) / (x_divisions - 1);
                    size_t idx = y * x_divisions + x;
                    m_Vertices.emplace_back(points[idx], Point2D<double>(u, v));

                    // Update min and max UV for bounding box
                    bbox.minimum.x = std::min(bbox.minimum.x, points[idx].x);
                    bbox.minimum.y = std::min(bbox.minimum.y, points[idx].y);
                    bbox.maximum.x = std::max(bbox.maximum.x, points[idx].x);
                    bbox.maximum.y = std::max(bbox.maximum.y, points[idx].y);
                }
            }
            m_BoundingBox = bbox;
            // Generate our octree for faster traversal during the sampling
            m_Octree = Octree<T>(m_BoundingBox, 16);

            // Create the half-edges and faces for each mesh
            m_Edges.reserve((x_divisions - 1) * (y_divisions - 1) * 4);    // 4 half-edges per quad
            m_Faces.reserve((x_divisions - 1) * (y_divisions - 1));        // 1 face per quad
            for (size_t y = 0; y < y_divisions - 1; ++y)
            {
                for (size_t x = 0; x < x_divisions - 1; ++x)
                {
                    size_t v0 = y * x_divisions + x;    // top-left vertex
                    size_t v1 = v0 + 1;                 // top-right vertex
                    size_t v2 = v0 + x_divisions;       // bottom-left vertex
                    size_t v3 = v2 + 1;                 // bottom-right vertex
                    std::array<size_t, 4> vert_indices = { v0, v1, v3, v2 };

                    auto& created_face = m_Faces.emplace_back();
                    created_face.vertex_indices({v0, v1, v2, v3});

                    // Create half-edges for the quad
                    size_t hePrev = m_Edges.size(); // Keep track of the previous half-edge
                    for (size_t i = 0; i < 4; ++i)
                    {
                        size_t heCurr = m_Edges.size(); // Current half-edge index
                        m_Edges.emplace_back(); // Create a new half-edge

                        m_Edges[heCurr].vertex(vert_indices[i]); // Set starting vertex
                        m_Edges[heCurr].pointed_at(vert_indices[(i + 1) % 4]); // Pointed-at vertex, wraps around for next one
                        m_Edges[heCurr].face(m_Faces.size()); // Set face

                        if (i > 0)
                        {
                            m_Edges[hePrev].next(heCurr); // Link previous half-edge to current
                        }
                        hePrev = heCurr; // Update previous half-edge index
                    }

                    // Complete the loop by linking the last half-edge to the first
                    m_Edges[hePrev].next(m_Edges.size() - 4); // Link last half-edge to first half-edge
                    created_face.half_edge(m_Edges.size() - 4); // Start with the first half-edge
                }
            }
        
            // Finally link all the half edges, this connects all the opposites
            link_half_edges();


            // Insert each face into the octree
            for (size_t i = 0; i < m_Faces.size(); ++i)
            {
                m_Octree.insert(*this, i);
            }

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

        const HalfEdge& half_edge(size_t index) const
        {
            if (index == std::numeric_limits<size_t>::max())
            {
                PSAPI_LOG_ERROR("Mesh", "Unable to retrieve half edge as its index is not valid");
            }
            if (index > m_Edges.size() - 1)
            {
                PSAPI_LOG_ERROR("Mesh", "Unable to retrieve half edge as its index is not valid, max allowed index %zu. Given index: %zu", m_Edges.size() - 1, index);
            }

            return m_Edges[index];
        }

        const std::vector<HalfEdge>& half_edges() const
        {
            return m_Edges;
        }

        const Face& face(size_t index) const
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

        const std::vector<Face>& faces() const
        {
            return m_Faces;
        }

        /// Look up the mesh uv coordinate at the given point returning {-1, -1} if the 
        /// point does not lie on the mesh or if the 
        Point2D<double> uv_coordinate(Point2D<T> position) const
        {
            if (!m_BoundingBox.in_bbox(position))
            {
                return Point2D<double>(-1.0, -1.0);
            }

            // Step 1: Identify the face (quad) containing `position`
            auto face_indices = m_Octree.query(position);
            for (const auto& face_index : face_indices)
            {
                const auto& face = m_Faces[face_index];
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

                // Check if the position is within this face (simple point-in-quad check)
                if (point_in_quad(position, v0.point(), v1.point(), v3.point(), v2.point()))
                {
                    // Bilinear interpolation for UVs
                    return bilinear_interpolation_uv(position, v0, v1, v3, v2);
                }
            }
            return Point2D<double>(-1.0, -1.0); // No valid UV coordinate found
        }

    private:

        bool point_in_triangle(const Point2D<T> p, const Point2D<T> a, const Point2D<T> b, const Point2D<T> c) const
        {
            auto sign = [](const Point2D<T> p1, const Point2D<T> p2, const Point2D<T> p3) 
            {
                return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
            };

            T d1 = sign(p, a, b);
            T d2 = sign(p, b, c);
            T d3 = sign(p, c, a);

            bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
            bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

            return !(has_neg && has_pos);
        }


        bool point_in_quad(const Point2D<T> p, const Point2D<T> v0, const Point2D<T> v1,
            const Point2D<T> v3, const Point2D<T> v2) const
        {
            // Check if the point is inside either of the triangles formed by splitting the quad
            return point_in_triangle(p, v0, v1, v3) || point_in_triangle(p, v0, v2, v3);
        }


        template<typename T>
        std::tuple<double, double, double> barycentric_coordinates(const Point2D<T> p, const Point2D<T> a, const Point2D<T> b, const Point2D<T> c) const
        {
            auto dot = [](const Point2D<T> a, const Point2D<T> b)
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


        Point2D<double> bilinear_interpolation_uv(const Point2D<T> p, const Vertex<T> v0, const Vertex<T> v1,
            const Vertex<T> v3, const Vertex<T> v2) const
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


        /// Link all the half edges to their opposites
        void link_half_edges()
        {
            // Map each edge to its index by storing vertex pairs
            std::unordered_map<std::pair<size_t, size_t>, size_t, pair_hash> edge_map;

            // First pass: Populate the map with edge pairs
            for (size_t i = 0; i < m_Edges.size(); ++i)
            {
                // Define each half-edge with (pointed_at_idx, vertex_idx) for easier reverse lookup
                auto key = std::make_pair(m_Edges[i].pointed_at_idx(), m_Edges[i].vertex_idx());
                edge_map[key] = i;
            }

            // Second pass: Link opposites
            for (size_t i = 0; i < m_Edges.size(); ++i)
            {
                // Define the opposite edge key (vertex_idx, pointed_at_idx)
                auto opposite_key = std::make_pair(m_Edges[i].vertex_idx(), m_Edges[i].pointed_at_idx());

                // Find the opposite half-edge in the map
                auto it = edge_map.find(opposite_key);
                if (it != edge_map.end() && it->second != i)
                {
                    // Link opposites
                    size_t j = it->second;
                    m_Edges[i].opposite(j);
                    m_Edges[j].opposite(i);

                    // Remove from map to avoid double-linking
                    edge_map.erase(it);
                }
            }
        }

        std::vector<Vertex<T>>  m_Vertices;
        std::vector<Face>       m_Faces;
        std::vector<HalfEdge>   m_Edges;
        BoundingBox<T>          m_BoundingBox;
        Octree<T>               m_Octree;

    };
}

PSAPI_NAMESPACE_END