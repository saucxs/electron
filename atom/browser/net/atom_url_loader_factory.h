// Copyright (c) 2019 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_BROWSER_NET_ATOM_URL_LOADER_FACTORY_H_
#define ATOM_BROWSER_NET_ATOM_URL_LOADER_FACTORY_H_

#include <string>

#include "mojo/public/cpp/bindings/binding_set.h"
#include "native_mate/dictionary.h"
#include "net/url_request/url_request_job_factory.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"

namespace atom {

// Old Protocol API can only serve one type of response for one scheme.
enum class ProtocolType {
  kBuffer,
  kString,
  kFile,
  kHttp,
  kStream,
  kFree,  // special type for returning arbitrary type of response.
};

using StartLoadingCallback =
    base::OnceCallback<void(v8::Local<v8::Value>, mate::Arguments*)>;
using ProtocolHandler =
    base::Callback<void(const network::ResourceRequest&, StartLoadingCallback)>;

// Implementation of URLLoaderFactory.
class AtomURLLoaderFactory : public network::mojom::URLLoaderFactory {
 public:
  AtomURLLoaderFactory(ProtocolType type, const ProtocolHandler& handler);
  ~AtomURLLoaderFactory() override;

  // network::mojom::URLLoaderFactory:
  void CreateLoaderAndStart(network::mojom::URLLoaderRequest loader,
                            int32_t routing_id,
                            int32_t request_id,
                            uint32_t options,
                            const network::ResourceRequest& request,
                            network::mojom::URLLoaderClientPtr client,
                            const net::MutableNetworkTrafficAnnotationTag&
                                traffic_annotation) override;
  void Clone(network::mojom::URLLoaderFactoryRequest request) override;

 private:
  static void StartLoading(
      network::mojom::URLLoaderRequest loader,
      int32_t routing_id,
      int32_t request_id,
      uint32_t options,
      const network::ResourceRequest& request,
      network::mojom::URLLoaderClientPtr client,
      const net::MutableNetworkTrafficAnnotationTag& traffic_annotation,
      ProtocolType type,
      v8::Local<v8::Value> response,
      mate::Arguments* args);
  static void StartLoadingBuffer(network::mojom::URLLoaderClientPtr client,
                                 const mate::Dictionary& dict);
  static void StartLoadingString(network::mojom::URLLoaderClientPtr client,
                                 const mate::Dictionary& dict,
                                 v8::Isolate* isolate,
                                 v8::Local<v8::Value> response);
  static void StartLoadingFile(network::mojom::URLLoaderRequest loader,
                               network::ResourceRequest request,
                               network::mojom::URLLoaderClientPtr client,
                               const mate::Dictionary& dict,
                               v8::Isolate* isolate,
                               v8::Local<v8::Value> response);
  static void StartLoadingHttp(
      network::mojom::URLLoaderRequest loader,
      int32_t routing_id,
      int32_t request_id,
      uint32_t options,
      const network::ResourceRequest& original_request,
      network::mojom::URLLoaderClientPtr client,
      const net::MutableNetworkTrafficAnnotationTag& traffic_annotation,
      const mate::Dictionary& dict);
  static void StartLoadingStream(network::mojom::URLLoaderRequest loader,
                                 network::mojom::URLLoaderClientPtr client,
                                 const mate::Dictionary& dict);

  // Helper to send string as response.
  static void SendContents(network::mojom::URLLoaderClientPtr client,
                           network::ResourceResponseHead head,
                           std::string data);

  // TODO(zcbenz): This comes from extensions/browser/extension_protocols.cc
  // but I don't know what it actually does, find out the meanings of |Clone|
  // and |bindings_| and add comments for them.
  mojo::BindingSet<network::mojom::URLLoaderFactory> bindings_;

  ProtocolType type_;
  ProtocolHandler handler_;

  DISALLOW_COPY_AND_ASSIGN(AtomURLLoaderFactory);
};

}  // namespace atom

#endif  // ATOM_BROWSER_NET_ATOM_URL_LOADER_FACTORY_H_
