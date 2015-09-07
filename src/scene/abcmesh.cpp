#include "abcmesh.h"

#include <GL/glew.h>

#include <opensubdiv/far/topologyDescriptor.h>
#include <opensubdiv/far/primvarRefiner.h>
#include <opensubdiv/far/stencilTableFactory.h>

using namespace OpenSubdiv;
typedef Far::TopologyDescriptor Descriptor;

AbcMesh::AbcMesh(const IPolyMesh &mesh, AbcSceneReader *reader) :m_time(0)
{
    m_mesh = mesh;
    m_schema = m_mesh.getSchema();
    m_schema.get(m_sample, m_time);
    subdiv_level = 0;
    m_refiner = NULL;
    m_reader = reader;

    // strip namespace
    std::string s = mesh.getName();
    std::string delimiter = ":";
    size_t pos = 0;
    std::string token;

    while ((pos = s.find(delimiter)) != std::string::npos) {
       token = s.substr(0, pos);
       s.erase(0, pos + delimiter.length());
    }
    name = s;
}

void AbcMesh::create()
{
    glGenVertexArrays(1, &vertexArrayId);
    glGenBuffers(1, &vertexBufferId);
    glGenBuffers(1, &uvBufferId);
    glGenBuffers(1, &normalBufferId);
    glGenBuffers(1, &indexBufferId);
    created = true;
    std::cerr << "created " << name << "\n";
}
AbcMesh::~AbcMesh()
{
    glDeleteBuffers(1, &vertexBufferId);
    glDeleteBuffers(1, &indexBufferId);
    glDeleteBuffers(1, &uvBufferId);
    glDeleteBuffers(1, &normalBufferId);

    glDeleteVertexArrays(1, &vertexArrayId);

    delete m_refiner;
}

void AbcMesh::update()
{
    //std::lock_guard<std::recursive_mutex> l(m_reader->lock);

    if (!created){
        create();
    }
    //std::vector<glm::vec3> vertices;
    //std::vector<glm::vec2> uvs;
    //std::vector<glm::vec3> normals;
    //std::vector<unsigned int> indices;



    MeshData mesh_data;
    read_data(mesh_data, time);
    //read(vertices, uvs, normals, indices);



    glBindVertexArray(vertexArrayId);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, mesh_data.vertices.size() * sizeof(glm::vec3),
                 &mesh_data.vertices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
       0,                  // attribute 0 = position
       3,                  // size
       GL_FLOAT,           // type
       GL_FALSE,           // normalized?
       0,                  // stride
       (void*)0            // array buffer offset
    );

    glBindBuffer(GL_ARRAY_BUFFER, uvBufferId);
    glBufferData(GL_ARRAY_BUFFER, mesh_data.uvs.size() * sizeof(glm::vec2),
                 &mesh_data.uvs[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
       1,                  // attribute 1 = position
       2,                  // size
       GL_FLOAT,           // type
       GL_FALSE,           // normalized?
       0,                  // stride
       (void*)0            // array buffer offset
    );

    glBindBuffer(GL_ARRAY_BUFFER, normalBufferId);
    glBufferData(GL_ARRAY_BUFFER, mesh_data.normals.size() * sizeof(glm::vec3),
                 &mesh_data.normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
       2,                  // attribute 2 = position
       3,                  // size
       GL_FLOAT,           // type
       GL_FALSE,           // normalized?
       0,                  // stride
       (void*)0            // array buffer offset
    );

    indexSize = mesh_data.indices.size();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * sizeof(unsigned int),
                 &mesh_data.indices[0] , GL_STATIC_DRAW);

    glBindVertexArray(0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

}
void AbcMesh::draw()
{

    if (!visible)
        return;

    glBindVertexArray(vertexArrayId);


    // Draw the triangle !
    glDrawElements(
                   GL_TRIANGLES,      // mode
                   indexSize,         // count
                   GL_UNSIGNED_INT,   // type
                   (void*)0           // element array buffer offset
                   );
    glBindVertexArray(0);
}

struct PackedVertex{
    glm::vec3 vertex;
    glm::vec2 uv;
    glm::vec3 normal;
    bool operator<(const PackedVertex that) const{
                    return memcmp((void*)this, (void*)&that, sizeof(PackedVertex))>0;
    };

};

bool get_similar_vertex_index(const PackedVertex & packed,
                              std::map<PackedVertex, unsigned int> & vertex_pack_map,
                              unsigned int & result)
{

    std::map<PackedVertex,unsigned int>::iterator it = vertex_pack_map.find(packed);
    if ( it == vertex_pack_map.end() ) {
        return false;
    } else {
        result = it->second;
        return true;
    }
}

struct Vertex {

    void Clear()
    {
        pos = {0.0f, 0.0f, 0.0f,};
    }

    void AddWithWeight(Vertex const & src, float weight) {
        pos += weight*src.pos;
    }

    glm::vec3 pos;
};

struct VertexUV {

    void Clear() {
        uv= {0.0f, 0.0f};
    }

    void AddWithWeight(VertexUV const & src, float weight) {
        uv += weight * src.uv;
    }

    glm::vec2 uv;
};

void AbcMesh::create_refiner()
{
    if (m_refiner) {
        delete m_refiner;
        m_refiner = NULL;
    }

    int maxlevel= subdiv_level ? subdiv_level: 1;

    OpenSubdiv::Sdc::SchemeType type = OpenSubdiv::Sdc::SCHEME_CATMARK;
    OpenSubdiv::Sdc::Options options;
    options.SetVtxBoundaryInterpolation(OpenSubdiv::Sdc::Options::VTX_BOUNDARY_EDGE_ONLY);
    //options.SetFVarLinearInterpolation(Sdc::Options::FVAR_LINEAR_CORNERS_PLUS2);
    options.SetFVarLinearInterpolation(Sdc::Options::FVAR_LINEAR_BOUNDARIES);

    const P3fArraySamplePtr &positions = m_sample.getPositions();
    const Int32ArraySamplePtr &faceIndices = m_sample.getFaceIndices();
    const Int32ArraySamplePtr &faceCounts = m_sample.getFaceCounts();

    Descriptor desc;

    desc.numVertices  = positions->size();
    desc.numFaces     = faceCounts->size();
    desc.numVertsPerFace = faceCounts->get();
    desc.vertIndicesPerFace  = faceIndices->get();

    int channelUV = 0;
    int channelNormal = 1;

    vector<int> fvar_indices(faceIndices->size());
    for (int i; i < fvar_indices.size(); i++){
        fvar_indices[i] = i;
    }

    Descriptor::FVarChannel channels[2];
    channels[channelUV].numValues = fvar_indices.size();
    channels[channelUV].valueIndices = &fvar_indices[0];

    channels[channelNormal].numValues = fvar_indices.size();
    channels[channelNormal].valueIndices = &fvar_indices[0];

    desc.numFVarChannels = 2;
    desc.fvarChannels = channels;

    m_refiner = Far::TopologyRefinerFactory<Descriptor>::Create(desc,
                            Far::TopologyRefinerFactory<Descriptor>::Options(type, options));

    Far::TopologyRefiner::UniformOptions refineOptions(maxlevel);
    refineOptions.fullTopologyInLastLevel = true;
    m_refiner->RefineUniform(refineOptions);

}

void AbcMesh::subdivide(std::vector<glm::vec3> &vertices,
                        std::vector<glm::vec2> &uvs,
                        std::vector<glm::vec3> &normals,
                        std::vector<unsigned int> &indices)
{
    int channelUV = 0;
    int channelNormal = 1;

    std::vector<glm::vec2> tmp_uvs;
    read_uvs(tmp_uvs);

    std::vector<glm::vec3> tmp_normals;
    read_normals(tmp_normals);

    int maxlevel= subdiv_level ? subdiv_level: 1;

    const P3fArraySamplePtr &positions = m_sample.getPositions();

    if (!m_refiner || m_refiner->GetMaxLevel() != maxlevel)
        create_refiner();

    int nCoarseVerts = positions->size();
    int nFineVerts   = m_refiner->GetLevel(maxlevel).GetNumVertices();
    int nTotalVerts  = m_refiner->GetNumVerticesTotal();
    int nTempVerts   = nTotalVerts - nCoarseVerts - nFineVerts;

    // Allocate intermediate and final storage to be populated:
    std::vector<Vertex> tempPosBuffer(nTempVerts);
    std::vector<Vertex> finePosBuffer(nFineVerts);

    Vertex * srcPos = (Vertex *) positions->get();
    Vertex * dstPos = &tempPosBuffer[0];

    int nCoarseVertUVs = tmp_uvs.size();
    int nFineVerUVs   = m_refiner->GetLevel(maxlevel).GetNumFVarValues(channelUV);
    int nTotalVertUVs  = m_refiner->GetNumFVarValuesTotal(channelUV);
    int nTempVertUVs   = nTotalVertUVs - nCoarseVertUVs - nFineVerUVs;

    //cerr << "fine sizes: " << nFineVerts << " "<< nFineVerUVs << "\n";

    std::vector<VertexUV> tempUVBuffer(nTempVertUVs);
    std::vector<VertexUV> fineUVBuffer(nFineVerUVs);

    VertexUV * srcUV = reinterpret_cast<VertexUV *>(&tmp_uvs[0]);
    VertexUV * dstUV = &tempUVBuffer[0];

    int nCoarseVertNormals = tmp_uvs.size();
    int nFineVerNormals   = m_refiner->GetLevel(maxlevel).GetNumFVarValues(channelNormal);
    int nTotalVertNormals  = m_refiner->GetNumFVarValuesTotal(channelNormal);
    int nTempVertNormals   = nTotalVertNormals - nCoarseVertNormals - nFineVerNormals;

    std::vector<Vertex> tempNormalBuffer(nTempVertNormals);
    std::vector<Vertex> fineNormalBuffer(nFineVerNormals);

    Vertex * srcNormal = reinterpret_cast<Vertex *>(&tmp_normals[0]);
    Vertex * dstNormal = &tempNormalBuffer[0];

    // Interpolate vertex primvar data
    OpenSubdiv::Far::PrimvarRefiner primvarRefiner(*m_refiner);

    for (int level = 1; level < maxlevel; ++level) {

        primvarRefiner.Interpolate(level, srcPos, dstPos);

        srcPos = dstPos;
        dstPos += m_refiner->GetLevel(level).GetNumVertices();

        primvarRefiner.InterpolateFaceVarying(level, srcUV, dstUV, channelUV);
        srcUV = dstUV;
        dstUV += m_refiner->GetLevel(level).GetNumFVarValues(channelUV);

        primvarRefiner.InterpolateFaceVarying(level, srcNormal, dstNormal, channelNormal);
        srcNormal = dstNormal;
        dstNormal += m_refiner->GetLevel(level).GetNumFVarValues(channelNormal);

    }

    primvarRefiner.Interpolate(maxlevel, srcPos, finePosBuffer);

    primvarRefiner.InterpolateFaceVarying(maxlevel, srcUV, fineUVBuffer, channelUV);
    primvarRefiner.InterpolateFaceVarying(maxlevel, srcNormal, fineNormalBuffer, channelNormal);

    Far::TopologyLevel const & refLastLevel = m_refiner->GetLevel(maxlevel);

    int nfaces = refLastLevel.GetNumFaces();


    std::map<PackedVertex, unsigned int> vertex_pack_map;

    int total_indice_count = 0;
    int total_optimised_count = 0;

    // Triangulation

    for (int face = 0; face < nfaces; ++face) {

        Far::ConstIndexArray fverts   = refLastLevel.GetFaceVertices(face);
        Far::ConstIndexArray fuvs     = refLastLevel.GetFaceFVarValues(face, channelUV);
        Far::ConstIndexArray fnormals = refLastLevel.GetFaceFVarValues(face, channelNormal);

        glm::ivec3 value_indices[3];
        value_indices[0] = glm::ivec3(fverts[0], fuvs[0], fnormals[0]);

        for (int vert = 1; vert < fverts.size() - 1; vert++) {

            value_indices[1] = glm::ivec3(fverts[vert    ], fuvs[vert    ], fnormals[vert    ]);
            value_indices[2] = glm::ivec3(fverts[vert + 1], fuvs[vert + 1], fnormals[vert + 1]);

            for (int j = 0; j < 3; j++) {\
                const PackedVertex packed = {finePosBuffer[   value_indices[j].x].pos,
                                             fineUVBuffer[    value_indices[j].y].uv,
                                             fineNormalBuffer[value_indices[j].z].pos
                                            };

                unsigned int found_index;
                bool found = get_similar_vertex_index(packed,
                                                      vertex_pack_map,
                                                      found_index);
                total_indice_count++;
                if (found) {
                    total_optimised_count++;
                    indices.push_back(found_index);
                } else {
                    vertices.push_back(packed.vertex);
                    uvs.push_back(packed.uv);
                    normals.push_back(packed.normal);
                    unsigned int new_index = vertices.size()-1;
                    indices.push_back(new_index);
                    vertex_pack_map[packed] = new_index;
                }
            }

        }


    }
    /*
    cerr <<  "Vertexs             : " << refLastLevel.GetNumVertices() << "\n";
    cerr <<  "FVar values uvs     : " << refLastLevel.GetNumFVarValues(channelUV) << "\n";
    cerr <<  "FVar values normals : " << refLastLevel.GetNumFVarValues(channelNormal) << "\n";

    cerr << "total " << total_indice_count << " opt to -> " << vertices.size() << "\n";
    */
}

void AbcMesh::read_data(MeshData &data, double time)
{
    std::lock_guard<std::recursive_mutex> l(m_reader->lock);
    m_schema.get(m_sample, time);
    read(data.vertices, data.uvs, data.normals, data.indices);
}

void AbcMesh::read(std::vector<glm::vec3> &vertices,
                   std::vector<glm::vec2> &uvs,
                   std::vector<glm::vec3> &normals,
                   std::vector<unsigned int> &indices)
{

    //cerr << "reading " << name << "\n";
    if (subdiv_level) {
        subdivide(vertices, uvs, normals, indices);
        return;
     }

    const P3fArraySamplePtr &positions = m_sample.getPositions();
    const Int32ArraySamplePtr &faceIndices = m_sample.getFaceIndices();
    const Int32ArraySamplePtr &faceCounts = m_sample.getFaceCounts();

    //cerr << "   vertex count: " << positions->size() << "\n";
    //cerr << "   indices: " << faceIndices->size() <<  "\n";
    //cerr << "   face count: " << faceCounts->size() << "\n";

    std::vector<glm::vec3> tmp_vertices;
    //read positions
    for(size_t i =0; i < positions->size(); i++) {
        const Alembic::AbcGeom::V3f p = positions->get()[i];
        tmp_vertices.push_back(glm::vec3(p.x, p.y, p.z));
    }

    std::vector<glm::vec2> tmp_uvs;
    read_uvs(tmp_uvs);
    //cerr << "   tmp uvs count: " << tmp_uvs.size() << "\n";

    std::vector<glm::vec3> tmp_normals;
    read_normals(tmp_normals);
    //cerr << "   tmp normals count: " << tmp_normals.size() << "\n";

    std::map<PackedVertex,unsigned int> vertex_pack_map;
    unsigned int cur_index = 0;

    // Triangulation

    for(size_t i =0; i < faceCounts->size(); i++) {
        std::pair<unsigned int, unsigned int> *p;
        std::pair<unsigned int, unsigned int> face_indices[3];

        int face_size = faceCounts->get()[i];
        p = &face_indices[0];
        p->first = cur_index;
        p->second = (unsigned int)(*faceIndices)[p->first];

        for (int j = 1; j < face_size -1; j++) {
            p = &face_indices[1];
            p->first = cur_index + j;
            p->second = (unsigned int)(*faceIndices)[p->first];

            p = &face_indices[2];
            p->first = cur_index + j + 1;
            p->second = (unsigned int)(*faceIndices)[p->first];

            for (int k = 0; k < 3; k++) {
                p = &face_indices[k];
                PackedVertex packed = {tmp_vertices[p->second],
                                       tmp_uvs[p->first],
                                       tmp_normals[p->first]
                                       };

                unsigned int found_index;
                bool found = get_similar_vertex_index(packed,
                                                      vertex_pack_map,
                                                      found_index);
                //found = false;
                if (found) {
                    indices.push_back(found_index);
                } else {
                    vertices.push_back(packed.vertex);
                    uvs.push_back(packed.uv);
                    normals.push_back(packed.normal);
                    unsigned int new_index = vertices.size()-1;
                    indices.push_back(new_index);
                    vertex_pack_map[packed] = new_index;
                }
            }

        }
        cur_index += face_size;
    }

    //cerr << "   out indices: " << indices.size() << "\n";
    //cerr << "   out uvs count: " << uvs.size() << "\n";
    //cerr << "   out normal count: " << normals.size() << "\n";
    //cerr << "   out vert count: " << vertices.size() << "\n";

}

void AbcMesh::read_uvs(std::vector<glm::vec2> &uvs) {

    size_t face_indices = m_sample.getFaceIndices()->size();
    IV2fGeomParam uv_param = m_schema.getUVsParam();
    if (!uv_param.valid()) {
        uvs.resize(face_indices);
        return;
    }

    IV2fGeomParam::Sample uv_sample(uv_param.getIndexedValue());
    if (!uv_sample.valid()){
        uvs.resize(face_indices);
        return;
    }

    const V2f *uv = uv_sample.getVals()->get();
    const size_t uv_count = uv_sample.getVals()->size();
    //cerr << "     reading uvs " << uv_count << "\n";

    if (uv_param.isIndexed()) {
        UInt32ArraySamplePtr indices_obj = uv_sample.getIndices();
        int indices_count = indices_obj->size();
        //cerr << "     mesh has indexed uvs = " << indices_count << "\n";

        for (size_t i = 0; i < indices_count; i++) {
             glm::uint32_t index = indices_obj->get()[i];
             const Alembic::AbcGeom::V2f v = uv[index];
             uvs.push_back(glm::vec2(v.x, v.y));
         }

    } else {

        for (size_t i = 0; i < uv_count; ++i) {
             const Alembic::AbcGeom::V2f v = uv[i];
             uvs.push_back(glm::vec2(v.x, v.y));
        }
    }
}

void AbcMesh::read_normals(std::vector<glm::vec3> &normals)
{
    IN3fGeomParam normal_param = m_schema.getNormalsParam();
    size_t face_indices = m_sample.getFaceIndices()->size();

    if (!normal_param.valid()) {
        create_normals(normals);
        return;
    }


    IN3fGeomParam::Sample normal_sample(normal_param.getIndexedValue());
    if (!normal_sample.valid()) {
        create_normals(normals);
        return;
    }

    const N3f* n = normal_sample.getVals()->get();
    const size_t normal_count = normal_sample.getVals()->size();

    //cerr << "     reading normals " << normal_count << "\n";

    if (normal_param.isIndexed()) {
        UInt32ArraySamplePtr indices_obj = normal_sample.getIndices();
        int indices_count = indices_obj->size();
       // cerr << "   mesh has indexed normals = " << indices_count << "\n";

        for (size_t i = 0; i < indices_count; i++) {
             glm::uint32_t index = indices_obj->get()[i];
             const Alembic::AbcGeom::N3f v = n[index];
             normals.push_back(glm::vec3(v.x, v.y, v.z));
         }
     } else {

        for (size_t i = 0; i < normal_count; ++i) {
            const Alembic::AbcGeom::N3f v = n[i];
            normals.push_back(glm::vec3(v.x, v.y, v.z));
        }
    }

}

struct SimpleVertex{
    glm::vec3 vertex;
    bool operator<(const SimpleVertex that) const{
                    return memcmp((void*)this, (void*)&that, sizeof(SimpleVertex))>0;
    };
};

void AbcMesh::create_normals(std::vector<glm::vec3> &normals)
{
    const P3fArraySamplePtr &positions = m_sample.getPositions();
    const Int32ArraySamplePtr &faceIndices = m_sample.getFaceIndices();
    const Int32ArraySamplePtr &faceCounts = m_sample.getFaceCounts();

    std::map< SimpleVertex, std::vector< std::pair < unsigned int , glm::vec3 > >  > vertex_map;
    std::map< SimpleVertex, std::vector< std::pair< unsigned int , glm::vec3 > > >::iterator it;

    //cacluate normals
    unsigned int cur_index = 0;
    for(int i =0; i < faceCounts->size(); i++) {
        int face_size = faceCounts->get()[i];

        // degenerated faces
        if (face_size < 3) {
            for(int j=0; j< face_size; j++) {
                normals.push_back(glm::vec3(0,0,0));
            }
            cur_index += face_size;
            continue;
        }

        glm::vec3 poly[3];

        // get just the first normal
        for(int j =0; j < 3; j++) {
            unsigned int index = (unsigned int)(*faceIndices)[cur_index + j];
            const Alembic::AbcGeom::V3f p = positions->get()[index];
            poly[j] = glm::vec3(p.x, p.y, p.z);
        }

        // caculate the normal
        glm::vec3 ab = poly[1] - poly[0];
        glm::vec3 ac = poly[2] - poly[0];
        glm::vec3 wn = glm::normalize(glm::cross(ac, ab));

        for(int j=0; j< face_size; j++) {
            unsigned int index = (unsigned int)(*faceIndices)[cur_index + j];
            const Alembic::AbcGeom::V3f p = positions->get()[index];
            SimpleVertex vert = {glm::vec3(p.x, p.y, p.z)};
            vertex_map[vert].push_back( std::pair< unsigned int ,
                                        glm::vec3>(cur_index + j, wn) );
        }

        cur_index += face_size;
    }

    // finally average the normals
    normals.resize(cur_index);
    for (it = vertex_map.begin(); it != vertex_map.end(); it++) {
        glm::vec3 a;
        for (int i = 0; i < it->second.size(); i ++) {
             a +=  it->second[i].second;
        }

        a /= it->second.size();
        for (int i = 0; i < it->second.size(); i ++) {
            normals[it->second[i].first] = a;
        }
    }


}
