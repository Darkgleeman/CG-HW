#include <accel.h>
#include <ray.h>
#include <geometry.h>

bool x_cmp(std::shared_ptr<Triangle> triangle1, std::shared_ptr<Triangle> triangle2)
{
    const vec3& t1_v0 = triangle1->getVertex(0);
    const vec3& t1_v1 = triangle1->getVertex(1);
    const vec3& t1_v2 = triangle1->getVertex(2);
    vec3 t1_center = (t1_v0 + t1_v1 + t1_v2) / 3;
    const vec3& t2_v0 = triangle2->getVertex(0);
    const vec3& t2_v1 = triangle2->getVertex(1);
    const vec3& t2_v2 = triangle2->getVertex(2);
    vec3 t2_center = (t2_v0 + t2_v1 + t2_v2) / 3;
    return t1_center[0] <= t2_center[0];
}

bool y_cmp(std::shared_ptr<Triangle> triangle1, std::shared_ptr<Triangle> triangle2)
{
    const vec3& t1_v0 = triangle1->getVertex(0);
    const vec3& t1_v1 = triangle1->getVertex(1);
    const vec3& t1_v2 = triangle1->getVertex(2);
    vec3 t1_center = (t1_v0 + t1_v1 + t1_v2) / 3;
    const vec3& t2_v0 = triangle2->getVertex(0);
    const vec3& t2_v1 = triangle2->getVertex(1);
    const vec3& t2_v2 = triangle2->getVertex(2);
    vec3 t2_center = (t2_v0 + t2_v1 + t2_v2) / 3;
    return t1_center[1] <= t2_center[1];
}

bool z_cmp(std::shared_ptr<Triangle> triangle1, std::shared_ptr<Triangle> triangle2)
{
    const vec3& t1_v0 = triangle1->getVertex(0);
    const vec3& t1_v1 = triangle1->getVertex(1);
    const vec3& t1_v2 = triangle1->getVertex(2);
    vec3 t1_center = (t1_v0 + t1_v1 + t1_v2) / 3;
    const vec3& t2_v0 = triangle2->getVertex(0);
    const vec3& t2_v1 = triangle2->getVertex(1);
    const vec3& t2_v2 = triangle2->getVertex(2);
    vec3 t2_center = (t2_v0 + t2_v1 + t2_v2) / 3;
    return t1_center[2] <= t2_center[2];
}
/**
 * construct kd tree with given triangles
 * @param triangles a array of triangles
 */
KdTreeNode::KdTreeNode(std::vector<std::shared_ptr<Triangle>> triangles,
                       AABB box, int depth) {
  // the space of the current node
  this->box = box;
  this->depth = depth;
  // stop dividing when the number of triangles is small enough
  // or stop when depth is too large
  // 8 and 40 are just examples, it may not be good enough
  if (triangles.size() < 8 || depth > 40) {
    // TODO: store triangles into the leaf node
      this->triangles = triangles;
      leftChild = nullptr;
      rightChild = nullptr;
    return;
  }

  // TODO: put the corresponding overlapping triangles
//为什么不能第一次就从左到右放置三角形，每次都要重新sort一遍，原因在于有可能三角形沿某个方向从左到右，换个方向就不是从左到右
  std::vector<std::shared_ptr<Triangle>> leftTriangles;
  std::vector<std::shared_ptr<Triangle>> rightTriangles;

  // TODO: divide the space into two parts according to one specific dimension
  //先通过depth%3决定选什么轴，然后遍历三角形找这三个点中心沿这个方向的坐标，确定split的位置。split和box最左边构成leftspace，和box最右边构成rightspace
  AABB leftSpace;
  AABB rightSpace;
  int axis = depth % 3;
  float median_value = (box.lb[axis] + box.ub[axis]) / 2+0.000001;
  leftSpace.lb = box.lb;
  leftSpace.ub = box.ub;
  //printf("axis:%d   meadin_value:%f\n", axis,median_value);
  //printf("before,leftSpace.ub:%f  %f  %f\n", leftSpace.ub[0], leftSpace.ub[1], leftSpace.ub[2]);
  leftSpace.ub[axis] = median_value;
  //printf("after,leftSpace.ub:%f  %f  %f\n", leftSpace.ub[0], leftSpace.ub[1], leftSpace.ub[2]);
  rightSpace.lb = box.lb;
  rightSpace.ub = box.ub;
  rightSpace.lb[axis] = median_value;
  for (auto& tri : triangles)
  {   
      AABB tri_box = AABB(tri->getVertex(0), tri->getVertex(1), tri->getVertex(2));
      if (leftSpace.isOverlap(tri_box))
          leftTriangles.push_back(tri);
      if (rightSpace.isOverlap(tri_box))
          rightTriangles.push_back(tri);
  }
  //std::vector<std::shared_ptr<Triangle>> temp_triangles(triangles.size());
  //std::copy(triangles.begin(), triangles.end(), temp_triangles.begin());
  //if (axis == 0)
  //{
  //    //choose the x-axis
  //    std::sort(temp_triangles.begin(), temp_triangles.end(),x_cmp);
  //    int half = temp_triangles.size();
  //    std::copy(temp_triangles.begin(), temp_triangles.begin() + half, std::back_inserter(leftTriangles));
  //    std::copy(temp_triangles.begin() + half, temp_triangles.end(), std::back_inserter(rightTriangles));
  //    for (auto& tri : leftTriangles)
  //        leftSpace = AABB(leftSpace,AABB(tri->getVertex(0), tri->getVertex(1), tri->getVertex(2)));
  //    for (auto& tri : rightTriangles)
  //        rightSpace = AABB(leftSpace, AABB(tri->getVertex(0), tri->getVertex(1), tri->getVertex(2)));
  //}
  //else if (axis == 1)
  //{
  //    //choose the y-axis
  //    std::sort(temp_triangles.begin(), temp_triangles.end(), y_cmp);
  //    int half = temp_triangles.size();
  //    std::copy(temp_triangles.begin(), temp_triangles.begin() + half, std::back_inserter(leftTriangles));
  //    std::copy(temp_triangles.begin() + half, temp_triangles.end(), std::back_inserter(rightTriangles));
  //    for (auto& tri : leftTriangles)
  //        leftSpace = AABB(leftSpace, AABB(tri->getVertex(0), tri->getVertex(1), tri->getVertex(2)));
  //    for (auto& tri : rightTriangles)
  //        rightSpace = AABB(leftSpace, AABB(tri->getVertex(0), tri->getVertex(1), tri->getVertex(2)));
  //}
  //else
  //{
  //    //choose the z-axis
  //    std::sort(temp_triangles.begin(), temp_triangles.end(), z_cmp);
  //    int half = temp_triangles.size();
  //    std::copy(temp_triangles.begin(), temp_triangles.begin() + half, std::back_inserter(leftTriangles));
  //    std::copy(temp_triangles.begin() + half, temp_triangles.end(), std::back_inserter(rightTriangles));
  //    for (auto& tri : leftTriangles)
  //        leftSpace = AABB(leftSpace, AABB(tri->getVertex(0), tri->getVertex(1), tri->getVertex(2)));
  //    for (auto& tri : rightTriangles)
  //        rightSpace = AABB(leftSpace, AABB(tri->getVertex(0), tri->getVertex(1), tri->getVertex(2)));
  //}


  // recursively build left and right
  leftChild = new KdTreeNode(leftTriangles, leftSpace, depth + 1);
  rightChild = new KdTreeNode(rightTriangles, rightSpace, depth + 1);
}

bool KdTreeNode::intersect(Interaction &interaction, const Ray &ray) const {
  // TODO: first check whether ray hit the bounding box
  Float tIn, tOut;
  if (!box.rayIntersection(ray, tIn, tOut)) return false;
  if (isLeaf()) {
    // all triangles are stored in leaf nodes
    // TODO: do intersect with triangles
      int flag = 0;
      for (std::shared_ptr<Triangle> temp_triangle : this->triangles) {
          Interaction it;
          if (temp_triangle->intersect(it, ray) && (interaction.entryDist == -1 || it.entryDist < interaction.entryDist)) {
              interaction = it;
              flag = 1;
          }
      }
      if (flag==1) return true;
      return false;
  }
  //printf("depth:%d\n", depth);

  // TODO: recursively test intersection with left and right
  bool left = leftChild->intersect(interaction, ray);
  bool right = rightChild->intersect(interaction, ray);
  return (left||right);
}

KdTreeNode::~KdTreeNode() {
  if (leftChild) {
    delete leftChild;
    leftChild = nullptr;
  }
  if (rightChild) {
    delete rightChild;
    rightChild = nullptr;
  }
}

KdTreeAccel::KdTreeAccel(
    const std::vector<std::shared_ptr<Triangle>> &triangles) {
  AABB box;
  for (auto &tri : triangles)
    box = AABB(box,
               AABB(tri->getVertex(0), tri->getVertex(1), tri->getVertex(2)));
  root = std::move(std::make_unique<KdTreeNode>(triangles, box, 0));
}

bool KdTreeAccel::intersect(Interaction &interaction, const Ray &ray) const {
  if (root) return root->intersect(interaction, ray);
  return false;
}

AABB::AABB(Float lbX, Float lbY, Float lbZ, Float ubX, Float ubY, Float ubZ) {
  lb = vec3(lbX, lbY, lbZ);
  ub = vec3(ubX, ubY, ubZ);
}

AABB::AABB(const vec3 &lb, const vec3 &ub) : lb(lb), ub(ub) {}

AABB::AABB(const vec3 &v1, const vec3 &v2, const vec3 &v3) {
  lb = v1.cwiseMin(v2).cwiseMin(v3);
  ub = v1.cwiseMax(v2).cwiseMax(v3);
}

AABB::AABB(const AABB &a, const AABB &b) {
  lb = vec3(a.lb.cwiseMin(b.lb));
  ub = vec3(a.ub.cwiseMax(b.ub));
}

vec3 AABB::getCenter() const { return (lb + ub) / 2; }

Float AABB::getDist(int c) const { return ub[c] - lb[c]; }

Float AABB::getVolume() const { return getDist(2) * getDist(1) * getDist(0); }

bool AABB::isOverlap(const AABB &a) const {
  return ((a.lb[0] >= this->lb[0] && a.lb[0] <= this->ub[0]) ||
          (this->lb[0] >= a.lb[0] && this->lb[0] <= a.ub[0])) &&
         ((a.lb[1] >= this->lb[1] && a.lb[1] <= this->ub[1]) ||
          (this->lb[1] >= a.lb[1] && this->lb[1] <= a.ub[1])) &&
         ((a.lb[2] >= this->lb[2] && a.lb[2] <= this->ub[2]) ||
          (this->lb[2] >= a.lb[2] && this->lb[2] <= a.ub[2]));
}

Float AABB::diagonalLength() const { return (ub - lb).norm(); }

bool AABB::rayIntersection(const Ray &ray, Float &tIn, Float &tOut) const {
  Float dirX = (ray.direction[0] == 0) ? INF : Float(1) / ray.direction[0];
  Float dirY = (ray.direction[1] == 0) ? INF : Float(1) / ray.direction[1];
  Float dirZ = (ray.direction[2] == 0) ? INF : Float(1) / ray.direction[2];
  //printf("lb[0]:%f\n", lb[0]);
  //printf("lb:%f  %f  %f\n", lb[0], lb[1], lb[2]);
  //printf("ub:%f  %f  %f\n", ub[0], ub[1], ub[2]);
  Float tx1 = (lb[0] - ray.origin[0]) * dirX;
  Float tx2 = (ub[0] - ray.origin[0]) * dirX;
  Float ty1 = (lb[1] - ray.origin[1]) * dirY;
  Float ty2 = (ub[1] - ray.origin[1]) * dirY;
  Float tz1 = (lb[2] - ray.origin[2]) * dirZ;
  Float tz2 = (ub[2] - ray.origin[2]) * dirZ;

  tIn = std::max(std::max(std::min(tx1, tx2), std::min(ty1, ty2)),
                 std::min(tz1, tz2));
  tOut = std::min(std::min(std::max(tx1, tx2), std::max(ty1, ty2)),
                  std::max(tz1, tz2));

  /* When tOut < 0 and the ray is intersecting with AABB, the whole AABB is
   * behind us */
  if (tOut < 0) {
    return false;
  }

  return tOut >= tIn;
}
