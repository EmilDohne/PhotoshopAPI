#pragma once

#include "Macros.h"

#include <vector>
#include <memory>
#include <cmath>

#include "BoundingBox.h"

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
    struct Mesh;
    template <typename T>
    struct Vertex;
    template <size_t _Size>
    struct Face;



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
        void insert(const Mesh<T>& mesh, size_t face_index, size_t depth, size_t max_depth)
        {
            const auto face_bbox = mesh.face(face_index).bbox(mesh);
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

        void insert(const Mesh<T>& mesh, size_t face_index)
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
        Face<4>& face(Mesh<T>& mesh)
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
        
        Point2D<T>& point()
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


    template <size_t _Size>
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

        std::array<size_t, _Size> vertex_indices()
        {
            return m_VertexIndices;
        }

    private:
        std::array<size_t, _Size> m_VertexIndices{};
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

        Mesh() = default;

        /// Generate a HalfEdgeMesh from a flat vector of points. These points must be in scanline order and the 
        /// generated mesh is quadrilateral.
        /// 
        /// The non affine transform describes rectangle with normalized positions 
        Mesh(const std::vector<Point2D<T>>& points, const std::array<Point2D<T>, 4> affine_transform, const std::array<Point2D<T>, 4> non_affine_transform, size_t x_divisions, size_t y_divisions)
        {
            initialize_mesh(points, affine_transform, non_affine_transform, x_divisions, y_divisions);
        }

        Mesh(const std::vector<Point2D<T>>& points, size_t x_divisions, size_t y_divisions)
        {
			auto bbox = BoundingBox<T>::compute(points);

			std::array<Point2D<T>, 4> affine_transform = {
				Point2D<T>{bbox.minimum.x, bbox.minimum.y}, Point2D<T>{bbox.maximum.x, bbox.minimum.y},
				Point2D<T>{bbox.minimum.x, bbox.maximum.y}, Point2D<T>{bbox.maximum.x, bbox.maximum.y}
			};

            std::array<Point2D<T>, 4> non_affine_transform = {
                Point2D<T>{static_cast<T>(0), static_cast<T>(0)}, Point2D<T>{static_cast<T>(1), static_cast<T>(0)},
                Point2D<T>{static_cast<T>(0), static_cast<T>(1)}, Point2D<T>{static_cast<T>(1), static_cast<T>(1)}
            };

            initialize_mesh(points, affine_transform, non_affine_transform, x_divisions, y_divisions);
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

        const Face<4>& face(size_t index) const
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

        const std::vector<Face<4>>& faces() const
        {
            return m_Faces;
        }


        /// Rotate the mesh around the center point, rebuilding the acceleration structure
        ///
        /// \param angle The angle to rotate by in radians
        /// \param center The center point to rotate about
        void rotate(double angle, Point2D<T> center)
        {
            double cos_theta = std::cos(angle);
            double sin_theta = std::sin(angle);

            for (auto& vertex : m_Vertices)
            {
                T x = vertex.point().x - center.x;
                T y = vertex.point().y - center.y;

                // Apply rotation around center point
                T rotated_x = x * cos_theta - y * sin_theta + center.x;
                T rotated_y = x * sin_theta + y * cos_theta + center.y;

                Point2D<T> rotated_point = { rotated_x, rotated_y };

                vertex.point() = rotated_point;
            }
            rebuild_bbox();
            rebuild_octree();
        }

        /// Scale the mesh around the center point, rebuilding the acceleration structure
        ///
        /// \param factor The scalar factor
        /// \param center The center point to scale about
        void scale(double factor, Point2D<T> center)
        {
            for (auto& vertex : m_Vertices)
            {
                T x = vertex.point().x - center.x;
                T y = vertex.point().y - center.y;

                Point2D<T> scaled_point = { x * factor + center.x, y * factor + center.y };
                // Apply scaling relative to the center
                vertex.point() = scaled_point;
            }
            rebuild_bbox();
            rebuild_octree();
        }

        /// Scale the mesh around the center point, rebuilding the acceleration structure
        ///
        /// \param scalar The scalar
        /// \param center The center point to scale about
        void scale(Point2D<T> scalar, Point2D<T> center)
        {
            for (auto& vertex : m_Vertices)
            {
                T x = vertex.point().x - center.x;
                T y = vertex.point().y - center.y;

                Point2D<T> scaled_point = { x * scalar.x + center.x, y * scalar.y + center.y };
                // Apply scaling relative to the center
                vertex.point() = scaled_point;
            }
            rebuild_bbox();
            rebuild_octree();
        }

        /// Move the mesh by the given offset, rebuilding the acceleration structure
        ///
        /// \param offset The offset to transform by
        void move(Point2D<T> offset)
        {
            for (auto& vertex : m_Vertices)
            {
                vertex.point().x += offset.x;
                vertex.point().y += offset.y;
            }
            rebuild_bbox();
            rebuild_octree();
        }


        /// Apply a 3x3 transformation matrix to the mesh, rebuilding the acceleration structure
        void transform(const Eigen::Matrix<T, 3, 3>& matrix, bool recalculate_bbox = true, bool recalculate_octree = true) 
        {
            for (auto& vertex : m_Vertices) 
            {
                Point2D<T> p = vertex.point();

                // Convert to homogeneous coordinates
                Eigen::Matrix<T, 3, 1> homogeneousPoint;
                homogeneousPoint << p.x, p.y, T(1.0);

                // Perform transformation
                Eigen::Matrix<T, 3, 1> transformedPoint = matrix * homogeneousPoint;

                T w = transformedPoint(2); // Get the homogeneous coordinate w

                if (w != static_cast<T>(0.0)) 
                {
                    vertex.point() = Point2D<T>(transformedPoint(0) / w, transformedPoint(1) / w);
                }
                else 
                {
                    PSAPI_LOG_ERROR("Mesh", "Error: tried to divide by zero");
                }
            }

            if (recalculate_bbox) {
                rebuild_bbox();
            }
            if (recalculate_octree) {
                rebuild_octree();
            }
        }

        static Eigen::Matrix<T, 3, 3> create_transformation_matrix(T moveX, T moveY, T angle, T scaleX, T scaleY, Point2D<T> pivot)
        {
            T cosAngle = std::cos(angle);
            T sinAngle = std::sin(angle);

            // Create the transformation matrix
            Eigen::Matrix<T, 3, 3> transformationMatrix;
            transformationMatrix <<
                scaleX * cosAngle, -scaleY * sinAngle, pivot.x + moveX - pivot.x * scaleX * cosAngle + pivot.y * scaleY * sinAngle,
                scaleX* sinAngle, scaleY* cosAngle, pivot.y + moveY - pivot.x * scaleX * sinAngle - pivot.y * scaleY * cosAngle,
                0, 0, 1;

            return transformationMatrix;
        }

        static Eigen::Matrix<T, 3, 3> create_transformation_matrix(Point2D<T> translate, T angle, Point2D<T> scale, Point2D<T> pivot)
        {
            return create_transformation_matrix(translate.x, translate.y, angle, scale.x, scale.y, pivot);
        }

        /// Compute a homography 3x3 transformation matrix to apply to our mesh based on the given source and destination quad.
        /// In most cases the src quad should be a unit quad 
        /// {0, 0}, {1, 0}
        /// {0, 1}, {1, 1}
        /// with the dst being the target coordinate of each of the source quads 
        static Eigen::Matrix3d create_homography_matrix(const std::array<Point2D<T>, 4>& source_points, const std::array<Point2D<T>, 4>& destination_points)
        {
            PSAPI_PROFILE_FUNCTION();
            // Populate the matrix using the src and dest coordinates
            // https://math.stackexchange.com/questions/494238/how-to-compute-homography-matrix-h-from-corresponding-points-2d-2d-planar-homog
            Eigen::MatrixXd A(8, 9);

            for (size_t i = 0; i < source_points.size(); ++i) 
            {
                double x = static_cast<double>(source_points[i].x);
                double y = static_cast<double>(source_points[i].y);
                double xw = static_cast<double>(destination_points[i].x);
                double yw = static_cast<double>(destination_points[i].y);

                A(2 * i, 0) = x;
                A(2 * i, 1) = y;
                A(2 * i, 2) = 1;
                A(2 * i, 3) = 0;
                A(2 * i, 4) = 0;
                A(2 * i, 5) = 0;
                A(2 * i, 6) = -xw * x;
                A(2 * i, 7) = -xw * y;
                A(2 * i, 8) = -xw;

                A(2 * i + 1, 0) = 0;
                A(2 * i + 1, 1) = 0;
                A(2 * i + 1, 2) = 0;
                A(2 * i + 1, 3) = x;
                A(2 * i + 1, 4) = y;
                A(2 * i + 1, 5) = 1;
                A(2 * i + 1, 6) = -yw * x;
                A(2 * i + 1, 7) = -yw * y;
                A(2 * i + 1, 8) = -yw;
            }

            // Compute A^T * A
            Eigen::MatrixXd AtA = A.transpose() * A;

            // Compute the eigenvalues and eigenvectors
            Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver(AtA);
            Eigen::VectorXd eigenvalues = solver.eigenvalues();
            Eigen::MatrixXd eigenvectors = solver.eigenvectors();

            // Get the index of the smallest eigenvalue
            int min_eig_idx = -1;
            eigenvalues.minCoeff(&min_eig_idx);

            // Extract the corresponding eigenvector
            Eigen::VectorXd smallest_eigen_vec = eigenvectors.col(min_eig_idx);

            // Reshape into 3x3 homography matrix
            Eigen::Matrix3d homography;
            homography << smallest_eigen_vec(0), smallest_eigen_vec(1), smallest_eigen_vec(2),
                smallest_eigen_vec(3), smallest_eigen_vec(4), smallest_eigen_vec(5),
                smallest_eigen_vec(6), smallest_eigen_vec(7), smallest_eigen_vec(8);

            // Normalize the homography
            homography /= homography(2, 2);

            return homography;
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
            for (size_t face_index : face_indices)
            {
                Face<4> face = m_Faces[face_index];
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

        BoundingBox<T> bbox() const noexcept { return m_BoundingBox; }

    private:

        std::vector<Vertex<T>>  m_Vertices;
        std::vector<Face<4>>    m_Faces;
        std::vector<HalfEdge>   m_Edges;
        BoundingBox<T>          m_BoundingBox;
        Octree<T, 16>           m_Octree;


        /// Check whether the non affine transform is a no op, i.e. if all the points are almost identical to a 
        /// Unit quad (within the epsilon). 
        bool non_affine_transform_is_noop(const std::array<Point2D<T>, 4> non_affine_transform, double epsilon = 1e-6) const
        {
            bool p0_same = Point2D<T>::equal(non_affine_transform[0], Point2D<T>(0, 0), epsilon);
            bool p1_same = Point2D<T>::equal(non_affine_transform[1], Point2D<T>(1, 0), epsilon);
            bool p2_same = Point2D<T>::equal(non_affine_transform[2], Point2D<T>(0, 1), epsilon);
            bool p3_same = Point2D<T>::equal(non_affine_transform[3], Point2D<T>(1, 1), epsilon);
            return p0_same && p1_same && p2_same && p3_same;
        }


        void _initialize_apply_transforms(BoundingBox<T> bbox, std::array<Point2D<T>, 4> affine_transform, std::array<Point2D<T>, 4> non_affine_transform)
        {
			// Apply the non affine transform in the form of a homography matrix recalculating the bounding box
			// during the transformation
			if (!non_affine_transform_is_noop(non_affine_transform))
			{
				std::array<Point2D<T>, 4> source_transform = {
					Point2D<T>(0, 0), Point2D<T>(1, 0),
					Point2D<T>(0, 1), Point2D<T>(1, 1)
				};

				auto bbox_size = m_BoundingBox.size();

				source_transform[0] *= bbox_size;
				source_transform[1] *= bbox_size;
				source_transform[2] *= bbox_size;
				source_transform[3] *= bbox_size;

				non_affine_transform[0] *= bbox_size;
				non_affine_transform[1] *= bbox_size;
				non_affine_transform[2] *= bbox_size;
				non_affine_transform[3] *= bbox_size;

				auto homography_matrix = Mesh<T>::create_homography_matrix(source_transform, non_affine_transform);
				transform(homography_matrix, true, false);
			}

			// Apply the affine transform, we do this after due to how we store our non-affine transform
			{
				std::array<Point2D<T>, 4> source_transform = {
					Point2D<T>(bbox.minimum.x, bbox.minimum.x), Point2D<T>(bbox.maximum.x, bbox.minimum.x),
					Point2D<T>(bbox.minimum.x, bbox.maximum.x), Point2D<T>(bbox.maximum.x, bbox.maximum.x)
				};

				auto homography_matrix = Mesh<T>::create_homography_matrix(source_transform, affine_transform);
                transform(homography_matrix, true, false);
			}
        }

        /// Initialize the mesh for a given number of points
        void initialize_mesh(
            const std::vector<Point2D<T>>& points, 
            std::array<Point2D<T>, 4> affine_transform,
            std::array<Point2D<T>, 4> non_affine_transform, 
            size_t x_divisions, 
            size_t y_divisions
        )
        {
            PSAPI_PROFILE_FUNCTION();

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

            _initialize_apply_transforms(bbox, affine_transform, non_affine_transform);

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
                    created_face.vertex_indices({ v0, v1, v2, v3 });

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

			// Generate our octree for faster traversal during the sampling
			m_Octree = Octree<T, 16>(m_BoundingBox, 16);
            for (size_t i = 0; i < m_Faces.size(); ++i)
            {
                m_Octree.insert(*this, i);
            }
        }


        void rebuild_bbox()
        {
            // Reset bounding box to extreme values
            m_BoundingBox.minimum = Point2D<T>(std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
            m_BoundingBox.maximum = Point2D<T>(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest());

            // Update the bounding box based on vertex positions
            for (const auto& vertex : m_Vertices)
            {
                m_BoundingBox.minimum.x = std::min(m_BoundingBox.minimum.x, vertex.point().x);
                m_BoundingBox.minimum.y = std::min(m_BoundingBox.minimum.y, vertex.point().y);
                m_BoundingBox.maximum.x = std::max(m_BoundingBox.maximum.x, vertex.point().x);
                m_BoundingBox.maximum.y = std::max(m_BoundingBox.maximum.y, vertex.point().y);
            }
        }


        void rebuild_octree()
        {
            // Rebuild the octree with updated positions
            m_Octree = Octree<T, 16>(m_BoundingBox);
            for (size_t i = 0; i < m_Faces.size(); ++i)
            {
                m_Octree.insert(*this, i);
            }
        }


        /// Compute whether a point is in a triangle defined by the three vertices vtx1, vtx2 and vtx3
        bool point_in_triangle(Point2D<T> point, Point2D<T> vtx1, Point2D<T> vtx2, Point2D<T> vtx3) const
        {
            auto sign = [](const Point2D<T> p1, const Point2D<T> p2, const Point2D<T> p3) 
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


        /// Link all the half edges to their opposites
        void link_half_edges()
        {
            PSAPI_PROFILE_FUNCTION();

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
    };
}

PSAPI_NAMESPACE_END